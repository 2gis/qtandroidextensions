/*
    QJniHelpers library for Qt

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

#pragma once

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QReadLocker>
#include <QtCore/QSet>
#include <QtCore/QSharedPointer>
#include <QtCore/QCoreApplication>
#include <QJniHelpers.h>
#include <QAndroidQPAPluginGap.h>
#include <IJniObjectLinker.h>



template <typename TNative>
class TJniObjectLinker
	: public IJniObjectLinker
{
public:
	TJniObjectLinker(TNative * nativePtr);
	virtual ~TJniObjectLinker();
	static TNative * getClient(jlong ptr);
	static QByteArray preloadJavaClasses();
	static QSharedPointer<QReadLocker> getLocker();

protected:
	QJniObject * handler() const;

private:
	QScopedPointer<QJniObject> handler_;
	jlong nativePtr_;

	static bool preloaded_;
	static QSet<jlong> clients_;
	static QReadWriteLock mutex_;
};


template <typename TNative> QSet<jlong>     TJniObjectLinker<TNative>::clients_;
template <typename TNative> QReadWriteLock  TJniObjectLinker<TNative>::mutex_(QReadWriteLock::Recursive);
template <typename TNative> bool            TJniObjectLinker<TNative>::preloaded_ = false;


template <typename TNative>
TJniObjectLinker<TNative>::TJniObjectLinker(TNative * nativePtr)
	: nativePtr_(jlong(nativePtr))
{
	{
		QWriteLocker locker(&mutex_);
		Q_ASSERT(!clients_.contains(nativePtr_));
		clients_.insert(nativePtr_);
	}

	try
	{
		QByteArray javaFullClassName = preloadJavaClasses();
		qDebug() << "Ready to create jni object" << javaFullClassName;
		handler_.reset(new QJniObject(javaFullClassName, "J", nativePtr_));

		if (!handler_->jObject())
		{
			qCritical() << "Failed to create " << javaFullClassName << " instance!";
			handler_.reset();
		}
	}
	catch (const std::exception & ex)
	{
		qCritical() << "Failed to preloadJavaClasses: " << ex.what();
	}
}


template <typename TNative>
TJniObjectLinker<TNative>::~TJniObjectLinker()
{
	{
		QWriteLocker locker(&mutex_);
		Q_ASSERT(clients_.contains(nativePtr_));
		clients_.remove(nativePtr_);
	}

	if (handler_)
	{
		try
		{
			handler_->callVoid("cppDestroyed");
			handler_.reset();
		}
		catch (const std::exception & ex)
		{
			qCritical() << "Failed to call cppDestroyed: " << ex.what();
		}
	}
}


template <typename TNative>
TNative * TJniObjectLinker<TNative>::getClient(jlong ptr)
{
	QReadLocker locker(&mutex_);

	if (clients_.contains(ptr))
	{
		return reinterpret_cast<TNative*>(ptr);
	}

	return NULL;
}


template <typename TNative>
QByteArray TJniObjectLinker<TNative>::preloadJavaClasses()
{
	QByteArray javaFullClassName;
	const JNINativeMethod * methods_list = NULL;
	size_t sizeof_methods_list = 0;
	TNative::getNativeMethods(javaFullClassName, &methods_list, sizeof_methods_list);

	QWriteLocker locker(&mutex_);

	try
	{
		if (!preloaded_)
		{
			preloaded_ = true;

			qDebug() << "Preloading jni for" << javaFullClassName;
			Q_ASSERT(!QCoreApplication::instance() || QCoreApplication::instance()->thread() == QThread::currentThread());

			QAndroidQPAPluginGap::preloadJavaClasses();
			QAndroidQPAPluginGap::preloadJavaClass(javaFullClassName);

			QJniClass ov(javaFullClassName);

			if (!ov.registerNativeMethods(methods_list, sizeof_methods_list))
			{
				qCritical() << "Failed to register native methods";
			}
		}
	}
	catch (const std::exception & ex)
	{
		preloaded_ = false;
		qCritical() << "Failed to preload jni for" << javaFullClassName << "with error message:" << ex.what();
		return "";
	}

	return javaFullClassName;
}


template <typename TNative>
QJniObject * TJniObjectLinker<TNative>::handler() const
{
	return handler_.data();
}


template <typename TNative>
QSharedPointer<QReadLocker> TJniObjectLinker<TNative>::getLocker()
{
	QSharedPointer<QReadLocker> locker(new QReadLocker(&mutex_));
	return locker;
}


#define JNI_LINKER_OBJECT(nativeClass, param, object)                                                                                       \
	QSharedPointer<QReadLocker> locker = nativeClass::JniObjectLinker::getLocker();                                                         \
	nativeClass * object = nativeClass::JniObjectLinker::getClient(param);                                                                  \
	if (NULL == object)                                                                                                                     \
	{                                                                                                                                       \
		qWarning() << "Failed to get native object for " #nativeClass;                                                                      \
		return;                                                                                                                             \
	}                                                                                                                                       \



#define JNI_LINKER_IMPL(nativeClass, java_class_name, methods)                                                                              \
                                                                                                                                            \
void nativeClass::preloadJavaClasses()                                                                                                      \
{                                                                                                                                           \
	JniObjectLinker::preloadJavaClasses();                                                                                                  \
}                                                                                                                                           \
                                                                                                                                            \
bool nativeClass::isJniReady() const                                                                                                        \
{                                                                                                                                           \
	return jniLinker_ && jniLinker_->handler() && jniLinker_->handler()->jObject();                                                         \
}                                                                                                                                           \
                                                                                                                                            \
QJniObject * nativeClass::jni() const                                                                                                       \
{                                                                                                                                           \
	Q_ASSERT(isJniReady());                                                                                                                 \
	return jniLinker_->handler();                                                                                                           \
}                                                                                                                                           \
                                                                                                                                            \
void nativeClass::getNativeMethods(QByteArray & javaFullClassName, const JNINativeMethod ** methods_list, size_t & sizeof_methods_list)     \
{                                                                                                                                           \
	javaFullClassName = java_class_name;                                                                                                    \
	*methods_list = methods;                                                                                                                \
	sizeof_methods_list = sizeof(methods);                                                                                                  \
}                                                                                                                                           \


