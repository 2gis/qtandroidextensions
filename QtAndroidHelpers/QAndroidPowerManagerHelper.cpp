/*
	Offscreen Android Views library for Qt

	Authors:
	Semen Zinkov <zinjkov.su@gmail.com>

	Distrbuted under The BSD License

	Copyright (c) 2021, DoubleGIS, LLC.
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

#include "QAndroidPowerManagerHelper.h"

#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>


Q_DECL_EXPORT void JNICALL Java_Provider_onInteractiveChanged(JNIEnv * env, jobject, jlong inst)
{
    Q_UNUSED(env);

    JNI_LINKER_OBJECT(QAndroidPowerManagerHelper, inst, proxy)
    proxy->onInteractiveChanged();
}


static const JNINativeMethod methods[] = {
    {"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
    {"onInteractiveChanged", "(J)V", reinterpret_cast<void *>(Java_Provider_onInteractiveChanged)},
};

JNI_LINKER_IMPL(QAndroidPowerManagerHelper, "ru/dublgis/androidhelpers/PowerManagerHelper", methods)

QAndroidPowerManagerHelper::QAndroidPowerManagerHelper(QObject * parent)
    : QObject(parent)
    , jniLinker_(new JniObjectLinker(this))
{
}

QAndroidPowerManagerHelper::~QAndroidPowerManagerHelper()
{
}

bool QAndroidPowerManagerHelper::isInteractive()
{
    if (isJniReady())
    {
        try
        {
            return jni()->callBool("isInteractive");
        }
        catch(const std::exception & ex)
        {
            qCritical() << "JNI exception in QAndroidPowerManagerHelper: " << ex.what();
        }
    }
    return false;
}

void QAndroidPowerManagerHelper::onInteractiveChanged()
{
    emit interactiveChanged();
}
