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

namespace  {

QStringList arrayListToQStringList(QJniObject * array_list)
{
	QStringList result;
	if (array_list && array_list->jObject())
	{
		jint size = array_list->callInt("size");
		QJniEnvPtr jep;
		for (jint i = 0; i < size; ++i)
		{
			QScopedPointer<QJniObject> str_object(array_list->callParamObject("get", "java/lang/Object", "I", i));
			if (str_object)
			{
				result << jep.JStringToQString(static_cast<jstring>(str_object->jObject()));
			}
			else
			{
				result << QString();
			}
		}
	}
	return result;
}

}

Q_DECL_EXPORT void JNICALL Java_ContactHelper_recievedContacts(JNIEnv * env, jobject, jlong param, jobject contactsList)
{
	if (!contactsList) 
	{
		qWarning() << __FUNCTION__ << "zero contacts";
		return;
	}

	ContactList allContacts;

	QJniObject jniContacts(contactsList, false);
	int size = jniContacts.callParamInt("size", "");
	allContacts.reserve(size);
	for (int i = 0; i < size; ++i)
	{
		QScopedPointer<QJniObject> jniContact(jniContacts.callParamObject(
			"get",
			"java/lang/Object",
			"I",
			i));

		if (jniContact)
		{
			Contact contact;
			contact.name = jniContact->getStringField("mFullName");

			QScopedPointer<QJniObject> numbers_array(jniContact->getObjectField("mPhones", "java/util/List"));
			contact.phones = arrayListToQStringList(numbers_array.data());

			QScopedPointer<QJniObject> emails_array(jniContact->getObjectField("mEmails", "java/util/List"));
			contact.emails = arrayListToQStringList(emails_array.data());

			allContacts.append(contact);
		}
	}

	void * vp = reinterpret_cast<void*>(param);
	QAndroidContacts * cppObject = reinterpret_cast<QAndroidContacts*>(vp);
	if (cppObject)
	{
		QMetaObject::invokeMethod(
			cppObject
			, "recievedContacts"
			, Qt::QueuedConnection
			, Q_ARG(ContactList, allContacts));
	}
}

static const JNINativeMethod methods[] = {
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
	{"nativeRecievedContacts", "(JLjava/lang/Object;)V", reinterpret_cast<void*>(Java_ContactHelper_recievedContacts)},
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

void QAndroidContacts::getContacts()
{
	try
	{
		if (isJniReady())
		{
			if (jni()->callBool("checkPermission"))
			{
				jni()->callVoid("readContacts");
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
