/*
	Android helper library for Qt

	Author:
	Sergey A. Galin <sergey.galin@gmail.com>

	Distrbuted under The BSD License

	Copyright (c) 2023, DoubleGIS, LLC.
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
#include "QAndroidExecutor.h"
#include <mutex>
#include <QtCore/QDebug>
#include <QtCore/QScopedPointer>
#include <QtCore/QThread>


namespace
{

const char * const c_helper_class_name = "ru/dublgis/androidhelpers/HandlerExecutor";

} // anonymous namespace


void QAndroidExecutor::preloadJavaClasses()
{
	static std::once_flag s_once;
	std::call_once(s_once, []() {
		try
		{
			QAndroidQPAPluginGap::preloadJavaClasses({
				"android/os/Handler",
				"android/os/Looper",
				c_helper_class_name
			});
			QJniClass(c_helper_class_name).registerNativeMethods({
				{"nativeCallback", "(J)V", reinterpret_cast<void*>(QAndroidExecutor::jcallback)},
			});
		}
		catch (const std::exception & e)
		{
			qCritical() << "preloadJavaClasses failed:" << e.what();
		}
	});
}


QAndroidExecutor::QAndroidExecutor(int exitWaitTimeMs)
	: exitWaitTimeMs_(exitWaitTimeMs)
{
	preloadJavaClasses();
	executor_ = createExecutor(createHandler(getMainThreadLooper()));
}


QAndroidExecutor::QAndroidExecutor(const QJniObject & handler, int exitWaitTimeMs)
	: exitWaitTimeMs_(exitWaitTimeMs)
{
	preloadJavaClasses();
	executor_ = createExecutor(handler);
}


QAndroidExecutor::~QAndroidExecutor()
{
	try
	{
		if (isValid())
		{
			if (exitWaitTimeMs_ > 0)
			{
				if (!wait(exitWaitTimeMs_))
				{
					qWarning() << "Failed to finish all tasks in" << exitWaitTimeMs_ << "ms!";
				}
			}
			else
			{
				QMutexLocker locker(&taskQueueMutex_);
				if (!tasks_.empty())
				{
					qWarning() << "Destroying executor with " << tasks_.size() << " pending tasks!";
				}
			}
			executor_.callVoid("terminate");
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "Exception in ~QAndroidExecutor():" << e.what();
	}
}


bool QAndroidExecutor::isCurrentThread()
{
	try
	{
		if (!isValid())
		{
			return false;
		}
		return executor_.callBool("isHandlerThread");
	}
	catch (const std::exception & e)
	{
		qCritical() << "Failed check for current thread:" << e.what();
		return false;
	}
}


// Post the task to the handler. The task will be executed asynchronously.
void QAndroidExecutor::post(Task && task)
{
	try
	{
		if (!task || !isValid())
		{
			return;
		}
		{
			QMutexLocker locker(&taskQueueMutex_);
			tasks_.push(task);
		}
		executor_.callVoid("execute");
	}
	catch (const std::exception & e)
	{
		qCritical() << "Exception in post():" << e.what();
	}
}


// Post the task to the handler, or, if we're already on the execution thread
// execute it immediately.
void QAndroidExecutor::execute(Task && task)
{
	try
	{
		if (task)
		{
			if (isCurrentThread())
			{
				task();
			}
			else
			{
				post(std::move(task));
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "Exception in execute():" << e.what();
	}
}


bool QAndroidExecutor::wait(int timeMs)
{
	// TODO: Rewrite using semaphore?
	for (int i = 0; i < timeMs; ++i)
	{
		{
			QMutexLocker locker(&taskQueueMutex_);
			if (tasks_.empty())
			{
				return true;
			}
		}
		QThread::msleep(1);
	}
	QMutexLocker locker(&taskQueueMutex_);
	return tasks_.empty();
}


QJniObject QAndroidExecutor::getMainThreadLooper()
{
	try
	{
		// TODO: Rewrite to non-pointer return funcion later
		// Java: context.getMainLooper()
		const QScopedPointer<QJniObject> looper(
			QAndroidQPAPluginGap::Context().callObject("getMainLooper", "android/os/Looper"));
		return (looper) ? *looper : QJniObject{};
	}
	catch (const std::exception & e)
	{
		qCritical() << "Failed to get main looper:" << e.what();
		return {};
	}
}


QJniObject QAndroidExecutor::createHandler(const QJniObject & looper)
{
	try
	{
		if (!looper)
		{
			return {};
		}
		// Java: new Handler(looper)
		return QJniObject("android/os/Handler",	"Landroid/os/Looper;", looper.jObject());
	}
	catch (const std::exception & e)
	{
		qCritical() << "Failed to create handler:" << e.what();
		return {};
	}
}


QJniObject QAndroidExecutor::createExecutor(const QJniObject & handler)
{
	try
	{
		if (!handler)
		{
			return {};
		}
		// Java: new HandlerExecutor(ptr, handler)
		return QJniObject(
			c_helper_class_name,
			"JLandroid/os/Handler;",
			reinterpret_cast<jlong>(this),
			handler.jObject());
	}
	catch (const std::exception & e)
	{
		qCritical() << "Failed to create executor:" << e.what();
		return {};
	}
}


void QAndroidExecutor::jcallback(JNIEnv *, jobject, jlong ptr)
{
	if (ptr)
	{
		reinterpret_cast<QAndroidExecutor*>(ptr)->callback();
	}
}


void QAndroidExecutor::callback()
{
	try
	{
		Task task = nullptr;
		{
			QMutexLocker locker(&taskQueueMutex_);
			if (tasks_.empty())
			{
				qCritical() << "Callback called with empty task queue!";
				return;
			}
			task = tasks_.front();
			tasks_.pop();
		}
		if (!task)
		{
			qCritical() << "Null task!";
			return;
		}
		task();
	}
	catch (const std::exception & e)
	{
		qCritical() << "Exception in task processor:" << e.what();
	}
}
