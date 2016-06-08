/*
  Offscreen Android Views library for Qt

  Author:
  Artöm R. Khvosch <a.khvosch@2gis.ru>

  Distrbuted under The BSD License

  Copyright (c) 2016, DoubleGIS, LLC.
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

#include <QAndroidQPAPluginGap.h>
#include <QJniHelpers.h>
#include "QNmeaListener.h"

static const char c_full_class_name[] = "ru/dublgis/androidgpslocation/NmeaListener";

Q_DECL_EXPORT void JNICALL Java_NmeaListener_OnNmeaReceivedNative(JNIEnv * env, jobject, jlong param, jlong timestamp, jstring str)
{
	try
	{
		if (param)
		{
			QNmeaListener * proxy = reinterpret_cast<QNmeaListener*>(reinterpret_cast<void*>(param));

			try
			{
				QJniEnvPtr env_ptr(env);
				emit proxy->nmeaMessage(static_cast<qint64>(timestamp), env_ptr.QStringFromJString(str));
			}
			catch (std::exception & e)
			{
				qWarning() << __FUNCTION__ << " exception: " << e.what();
			}
		}
		else
		{
			qWarning() << __FUNCTION__ << "Zero param!";
		}
	}
	catch (std::exception & e)
	{
		qWarning() << __FUNCTION__ << " exception: " << e.what();
	}
}

static const JNINativeMethod methods[] = {
	{"getActivity", "()Landroid/app/Activity;", (void*)QAndroidQPAPluginGap::getActivity},
	{"getContext", "()Landroid/content/Context;", (void*)QAndroidQPAPluginGap::getCurrentContext},
	{"OnNmeaReceivedNative", "(JJLjava/lang/String;)V", (void*)Java_NmeaListener_OnNmeaReceivedNative},
};

QNmeaListener::QNmeaListener(QObject * parent)
	: QObject(parent)
	, JniObjectLinker(reinterpret_cast<void*>(this), c_full_class_name, methods, sizeof(methods))
{
	handler()->callVoid("StartListening");
}

QNmeaListener::~QNmeaListener()
{
	handler()->callVoid("StopListening");
}
