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
	finish();
}


bool QAndroidExecutor::isValid() const
{
	return !!executor_ && !finished_;
}


bool QAndroidExecutor::isExecutionThread()
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
	if (!task)
	{
		qWarning() << "post() called for null task";
		return;
	}
	try
	{
		QMutexLocker locker(&mainMutex_);
		if (!isValid())
		{
			return;
		}
		if (tasks_.empty())
		{
			hasPendingTasksMutex_.lock();
		}
		tasks_.push(task);
		// Number of scheduled callbacks equals number of scheduled tasks
		executor_.callVoid("scheduleCallback");
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
	if (!task)
	{
		qWarning() << "execute() called for null task";
		return;
	}
	try
	{
		QMutexLocker locker(&mainMutex_);
		if (!isValid())
		{
			return;
		}
		if (isExecutionThread())
		{
			task();
		}
		else
		{
			post(std::move(task));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "Exception in execute():" << e.what();
	}
}


// private
bool QAndroidExecutor::wait(int waitTimeMs)
{
	QMutexLocker locker(&mainMutex_);
	// If we're in a broken condition we should not wait for tasks to be executed.
	// Also, we cannot verify if we're on the execution thread.
	if (!isValid())
	{
		return tasks_.empty();
	}
	// If we do tryLock() on the execution thread we would block it from executing the tasks
	// and only waste time until timeout.
	if (!isExecutionThread())
	{
		if (!tasks_.empty() && waitTimeMs > 0)
		{
			if (hasPendingTasksMutex_.tryLock(waitTimeMs))
			{
				// The mutexes job is done, do not leave it locked behind
				hasPendingTasksMutex_.unlock();
			}
		}
	}
	else
	{
		qCritical() << "wait() should not be called on the execution thread!";
	}
	return tasks_.empty();
}


// private
void QAndroidExecutor::dropQueue()
{
	QMutexLocker locker(&mainMutex_);
	if (!tasks_.empty())
	{
		qDebug() << "Dropping" << tasks_.size() << "pending tasks.";
		// Kudos to C++ commitee for not adding std::queue::reset() ;)
		TaskQueue empty;
		tasks_.swap(empty);
		hasPendingTasksMutex_.unlock();
	}
}


bool QAndroidExecutor::finish(int waitTimeMs)
{
	// No new tasks can be added while this mutex is locked
	QMutexLocker locker(&mainMutex_);
	if (finished_)
	{
		qWarning() << "finish() called twice!";
		return true;
	}
	const bool noDroppedTasks = (waitTimeMs > 0) ? wait(waitTimeMs) : tasks_.empty();
	// Throw away pending tasks still left, if any
	if (!noDroppedTasks)
	{
		dropQueue();
	}
	try
	{
		// Disable all furher callbacks from the Handler
		executor_.callVoid("terminate");
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in call to terminate():" << e.what();
	}
	// No new tasks can be added after this
	finished_= true;
	return noDroppedTasks;
}


// static
QJniObject QAndroidExecutor::getMainThreadLooper()
{
	try
	{
		// TODO: Rewrite to non-pointer return function later
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


// public static
QJniObject QAndroidExecutor::createHandler(const QJniObject & looper)
{
	try
	{
		if (!looper)
		{
			qWarning() << "Trying to create handler for null looper!";
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


// private
QJniObject QAndroidExecutor::createExecutor(const QJniObject & handler)
{
	try
	{
		if (!handler)
		{
			qWarning() << "Trying to create executor for null handler!";
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


// private static
void QAndroidExecutor::jcallback(JNIEnv *, jobject, jlong ptr)
{
	if (ptr)
	{
		reinterpret_cast<QAndroidExecutor*>(ptr)->callback();
	}
}


// private
void QAndroidExecutor::callback()
{
	try
	{
		Task task = nullptr;
		{
			QMutexLocker locker(&mainMutex_);
			if (tasks_.empty())
			{
				qCritical() << "Callback called with empty task queue!";
				return;
			}
			// Kudos for C++ committee for no function to move the element out ;)
			task = tasks_.front();
			tasks_.pop();
			if (tasks_.empty())
			{
				hasPendingTasksMutex_.unlock();
			}
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
