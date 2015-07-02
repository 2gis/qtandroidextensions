/*
	Offscreen Android Views library for Qt

	Author:
	Vyacheslav O. Koscheev <vok1980@gmail.com>

	Distrbuted under The BSD License

	Copyright (c) 2015, DoubleGIS, LLC.
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
	
#include "QSharedPreferences.h"

#include <QAndroidQPAPluginGap.h>


static const char * const c_full_class_name_ = "ru/dublgis/androidhelpers/SharedPreferencesHelper";



QSharedPreferences::QSharedPreferences(QObject * parent /*= 0*/) : 
	QObject(parent)
{
	preloadJavaClasses();

	// Creating Java object
	java_handler_.reset(new QJniObject(c_full_class_name_, "J",
		jlong(reinterpret_cast<void*>(this))));
}


QSharedPreferences::~QSharedPreferences()
{
	if (java_handler_)
	{
		java_handler_->callVoid("cppDestroyed");
		java_handler_.reset();
	}
}


void QSharedPreferences::preloadJavaClasses()
{
	static volatile bool preloaded_ = false;

	if (!preloaded_)
	{
		preloaded_ = true;

		QAndroidQPAPluginGap::preloadJavaClasses();
		QAndroidQPAPluginGap::preloadJavaClass(c_full_class_name_);

		QJniClass ov(c_full_class_name_);
		static const JNINativeMethod methods[] = 
		{
			{"getActivity", "()Landroid/app/Activity;", (void*)QAndroidQPAPluginGap::getActivity},
		};
	
		ov.registerNativeMethods(methods, sizeof(methods));
	}
}


void QSharedPreferences::writeString(const QString &key, const QString &value)
{
	QJniEnvPtr jep;
	java_handler_->callParamVoid("WriteString", "Ljava/lang/String;Ljava/lang/String;", 
										QJniLocalRef(jep, key).jObject(), QJniLocalRef(jep, value).jObject());
}


QString QSharedPreferences::readString(const QString & key, const QString & valueDefault)
{
	QJniEnvPtr jep;
	QString ret;

	try 
	{
		ret = java_handler_->callParamString("ReadString", "Ljava/lang/String;Ljava/lang/String;", 
										QJniLocalRef(jep, key).jObject(), QJniLocalRef(jep, valueDefault).jObject());

	}
	catch (const std::exception & e)
	{
		qWarning() << "Exception happend: " << e.what();
	}

	return ret;
}


void QSharedPreferences::writeInt(const QString & key, int32_t value)
{
	QJniEnvPtr jep;
	java_handler_->callParamVoid("WriteInt", "Ljava/lang/String;I", 
										QJniLocalRef(jep, key).jObject(), value);
}


int32_t QSharedPreferences::readInt(const QString & key, int32_t valueDefault)
{
	QJniEnvPtr jep;
	return java_handler_->callParamInt("ReadInt", "Ljava/lang/String;I", 
										QJniLocalRef(jep, key).jObject(), valueDefault);
}

