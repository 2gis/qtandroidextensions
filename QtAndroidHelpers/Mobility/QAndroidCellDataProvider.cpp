/*
	Offscreen Android Views library for Qt

	Authors:
		Vyacheslav O. Koscheev <vok1980@gmail.com>
		Ivan Avdeev marflon@gmail.com
		Sergey A. Galin sergey.galin@gmail.com

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

#include "QAndroidCellDataProvider.h"

#include <QtCore/QtDebug>
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>


namespace Mobility {


Q_DECL_EXPORT void JNICALL Java_CellListener_cellUpdate(JNIEnv *, jobject, jlong native_ptr, jstring type, jint cid, jint lac, jint mcc, jint mnc, jint rssi, jint ta)
{
	JNI_LINKER_OBJECT(Mobility::QAndroidCellDataProvider, native_ptr, proxy)
	proxy->cellUpdate(type, cid, lac, mcc, mnc, rssi, ta);
}


Q_DECL_EXPORT void JNICALL Java_CellListener_onSignalChanged(JNIEnv *, jobject, jlong native_ptr)
{
	JNI_LINKER_OBJECT(Mobility::QAndroidCellDataProvider, native_ptr, proxy)
	proxy->onSignalChanged();
}


static const JNINativeMethod methods[] = {
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
	{"onSignalChanged", "(J)V", reinterpret_cast<void*>(Java_CellListener_onSignalChanged)},
	{"cellUpdate", "(JLjava/lang/String;IIIIII)V", reinterpret_cast<void*>(Java_CellListener_cellUpdate)},
};


JNI_LINKER_IMPL(QAndroidCellDataProvider, "ru/dublgis/androidhelpers/mobility/CellListener", methods)


QAndroidCellDataProvider::QAndroidCellDataProvider(QObject * parent /*= 0*/)
	: QObject(parent)
	, jniLinker_(new JniObjectLinker(this))
{
}


QAndroidCellDataProvider::~QAndroidCellDataProvider()
{
}


void QAndroidCellDataProvider::start()
{
	try
	{
		if (isJniReady())
		{
			jni()->callBool("start");
		}
	}
	catch (const std::exception & ex)
	{
		qCritical() << "JNI exception in QAndroidCellDataProvider:" << ex.what();
	}
}


void QAndroidCellDataProvider::stop()
{
	try
	{
		if (isJniReady())
		{
			jni()->callVoid("stop");
		}
	}
	catch (const std::exception & ex)
	{
		qCritical() << "JNI exception in QAndroidCellDataProvider:" << ex.what();
	}
}


void QAndroidCellDataProvider::onSignalChanged()
{
	emit updated();
}


void QAndroidCellDataProvider::requestData()
{
	current_data_ = CellDataPtr::create();

	try
	{
		if (isJniReady())
		{
			jni()->callVoid("requestData");

			{
				QWriteLocker locker(&lock_data_);
				last_data_ = current_data_;
			}

			emit dataReady();
		}
	}
	catch (const std::exception & ex)
	{
		qCritical() << "JNI exception in QAndroidCellDataProvider:" << ex.what();
	}
}


void QAndroidCellDataProvider::cellUpdate(jstring type, jint cid, jint lac, jint mcc, jint mnc, jint rssi, jint ta)
{
	if (current_data_ && cid > 0 && cid != CellData::java_integer_max_value)
	{
		current_data_->data_.push_back(CellData::Data(cid));

		if (isJniReady())
		{
			QJniEnvPtr jep;
			current_data_->data_.back().radio_type_ = jep.JStringToQString(type);
		}

		current_data_->data_.back().location_area_code_ = lac;
		current_data_->data_.back().mobile_country_code_ = mcc;
		current_data_->data_.back().mobile_network_code_ = mnc;
		current_data_->data_.back().signal_strength_ = rssi;
		current_data_->data_.back().timing_advance_ = ta;
	}
}


CellDataPtr QAndroidCellDataProvider::getLastData()
{
	QReadLocker locker(&lock_data_);
	return last_data_;
}

} // namespace Mobility
