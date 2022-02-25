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


Q_DECL_EXPORT void JNICALL Java_ContactHelper_receivedContacts(JNIEnv * env, jobject, jlong param, jobject contacts)
{
	JNI_LINKER_OBJECT(QAndroidContacts, param, obj)
	ContactList contactList;

	try {
		QJniObject jniContactList(contacts, false);
		const int contactListSize = jniContactList.callParamInt("size", "");
		contactList.reserve(contactListSize);

		for (int contactIdx = 0; contactIdx < contactListSize; ++contactIdx)
		{
			QScopedPointer<QJniObject> jniContact(jniContactList.callParamObject(
				"get",
				"java/lang/Object",
				"I",
				contactIdx));

			if (jniContact)
			{
				Contact contact;
				contact.id = jniContact->callString("getId");
				contact.name = jniContact->callString("getFullName");

				QScopedPointer<QJniObject> jniEmailList(jniContact->callObject("getEmails", "java/util/List"));
				const int emailListSize = jniEmailList->callParamInt("size", "");
				contact.emails.reserve(emailListSize);

				for (int emailIdx = 0; emailIdx < emailListSize; ++emailIdx)
				{
					QScopedPointer<QJniObject> jniEmail(jniEmailList->callParamObject(
						"get",
						"java/lang/Object",
						"I",
						emailIdx));

					if (jniEmail)
					{
						Email email;
						email.label = jniEmail->callString("getLabel");
						email.address = jniEmail->callString("getAddress");

						contact.emails.append(email);
					}
				}

				QScopedPointer<QJniObject> jniPhoneList(jniContact->callObject("getPhones", "java/util/List"));
				const int phoneListSize = jniPhoneList->callParamInt("size", "");
				contact.phones.reserve(phoneListSize);

				for (int phoneIdx = 0; phoneIdx < phoneListSize; ++phoneIdx)
				{
					QScopedPointer<QJniObject> jniPhone(jniPhoneList->callParamObject(
						"get",
						"java/lang/Object",
						"I",
						phoneIdx));

					if (jniPhone)
					{
						Phone phone;
						phone.label = jniPhone->callString("getLabel");
						phone.number = jniPhone->callString("getNumber");

						contact.phones.append(phone);
					}
				}

				contactList.append(contact);
			}
		}
	}
	catch (const std::exception & e)
	{
		qWarning() << "JNI exception in " << __FUNCTION__ << ": " << e.what();
	}

	void * vp = reinterpret_cast<void*>(param);
	QAndroidContacts * cppObject = reinterpret_cast<QAndroidContacts*>(vp);
	if (cppObject)
	{
		QMetaObject::invokeMethod(
			cppObject,
			"receivedContacts",
			Qt::QueuedConnection,
			Q_ARG(ContactList, contactList));
	}
}

static const JNINativeMethod methods[] = {
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
	{"nativeReceivedContacts", "(JLjava/lang/Object;)V", reinterpret_cast<void*>(Java_ContactHelper_receivedContacts)},
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
				emit needReadContactPermission();
			}
		}
	}
	catch (const std::exception & e)
	{
		qWarning() << "JNI exception in " << __FUNCTION__ << ": " << e.what();
	}
}
