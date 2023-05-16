/*
  Helper for getting android contact book for Qt

  Author:
  Aleksey I. Gribanov <a.gribanov@2gis.ru>

  Distrbuted under The BSD License

  Copyright (c) 2022, DoubleGIS, LLC.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the DoubleGIS, LLC nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "QAndroidContacts.h"

#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>
#include <QtCore/QDebug>


namespace {

QVector<Email> toEmailList(QJniObject & jniEmailList)
{
	QVector<Email> emailList;

	if (!jniEmailList)
	{
		return emailList;
	}

	const int emailListSize = jniEmailList.callParamInt("size", "");
	emailList.reserve(emailListSize);

	for (int emailIdx = 0; emailIdx < emailListSize; ++emailIdx)
	{
		QJniObject jniEmail(jniEmailList.callParamObj(
			"get",
			"java/lang/Object",
			"I",
			emailIdx));

		if (!jniEmail)
		{
			continue;
		}

		emailList.append(Email{
			jniEmail.callString("getLabel"),
			jniEmail.callString("getAddress"),
		});
	}

	return emailList;
}

QVector<Phone> toPhoneList(QJniObject & jniPhoneList)
{
	QVector<Phone> phoneList;

	if (!jniPhoneList)
	{
		return phoneList;
	}

	const int phoneListSize = jniPhoneList.callParamInt("size", "");
	phoneList.reserve(phoneListSize);

	for (int phoneIdx = 0; phoneIdx < phoneListSize; ++phoneIdx)
	{
		QJniObject jniPhone(jniPhoneList.callParamObj(
			"get",
			"java/lang/Object",
			"I",
			phoneIdx));

		if (!jniPhone)
		{
			continue;
		}

		phoneList.append(Phone{
			jniPhone.callString("getLabel"),
			jniPhone.callString("getNumber"),
		});
	}

	return phoneList;
}

} // anonymous namespace


Q_DECL_EXPORT void JNICALL Java_ContactsHelper_contactsReceived(JNIEnv * env, jobject, jlong nativePtr, jobject contacts)
{
	JNI_LINKER_OBJECT(QAndroidContacts, nativePtr, contactsProxy)

	ContactList contactList;

	try
	{
		QJniObject jniContactList{contacts, false};
		const int contactListSize = jniContactList.callParamInt("size", "");
		contactList.reserve(contactListSize);

		for (int contactIdx = 0; contactIdx < contactListSize; ++contactIdx)
		{
			try
			{
				QJniObject jniContact{
					jniContactList.callParamObj("get", "java/lang/Object", "I", contactIdx)};

				if (!jniContact)
				{
					continue;
				}

				QJniObject jniEmailList{
					jniContact.callObj("getEmails", "java/util/List")};

				QJniObject jniPhoneList{
					jniContact.callObj("getPhones", "java/util/List")};

				contactList.append(Contact{
					jniContact.callString("getId"),
					jniContact.callString("getFullName"),
					toEmailList(jniEmailList),
					toPhoneList(jniPhoneList),
				});
			}
			catch (const std::exception & e)
			{
				qCritical() << __FUNCTION__ << ": JNI exception while getting contact info - " << e.what();
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << __FUNCTION__ << ": JNI exception while getting contact list - " << e.what();
	}

	const bool methodInvoked = QMetaObject::invokeMethod(
		contactsProxy,
		"contactsReceived",
		Qt::QueuedConnection,
		Q_ARG(ContactList, contactList));

	if (!methodInvoked)
	{
		qCritical() << __FUNCTION__ << ": QMetaObject::invokeMethod failed";
	}
}

static const JNINativeMethod methods[] = {
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
	{"nativeContactsReceived", "(JLjava/lang/Object;)V", reinterpret_cast<void*>(Java_ContactsHelper_contactsReceived)},
};

JNI_LINKER_IMPL(QAndroidContacts, "ru/dublgis/androidhelpers/Contacts", methods)

QAndroidContacts::QAndroidContacts(QObject * parent)
	: QObject(parent)
	, jniLinker_(new JniObjectLinker(this))
{
}

QAndroidContacts::~QAndroidContacts()
{
}

void QAndroidContacts::requestContacts()
{
	try
	{
		if (isJniReady())
		{
			if (jni()->callBool("checkPermission"))
			{
				jni()->callVoid("requestContacts");
			}
			else
			{
				emit contactsPermissionRequired();
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in " << __FUNCTION__ << ": " << e.what();
	}
}
