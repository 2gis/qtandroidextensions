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
#pragma once
#include <queue>
#include <QtCore/QMutex>
#include <QJniHelpers/QAndroidQPAPluginGap.h>


class QAndroidExecutor
{
public:
	static void preloadJavaClasses();

	using Task = std::function<void()>;

	// Create executor for main thread looper (the context is retrieved using
	// QAndroidQPAPluginGap, will work either in Activity or in service).
	// exitWaitTimeMs gives timeout for tasks to complete in destructor.
	QAndroidExecutor(int exitWaitTimeMs = 0);

	// Create executor for custom handler.
	QAndroidExecutor(const QJniObject & handler, int exitWaitTimeMs = 0);

	// Use QAndroidExecutor(QAndroidExecutor::createHandler(looper)) to create
	// for custom looper.
	static QJniObject createHandler(const QJniObject & looper);

	~QAndroidExecutor();

	bool isCurrentThread();
	bool isValid() const { return !!executor_; }

	// Post the task to the handler. Posted tasks will be executed asynchronously
	// in order of posting.
	void post(Task && task);

	// Post the task to the handler, or, if we're already on the execution thread
	// execute it immediately. Tasks that are executed from different threads
	// may be executed out of order.
	void execute(Task && task);

	// Wait for empty queue for specified time period. Returns true is the queue was
	// successfully emptied. Note: if the object is used from multiple threads some
	// other thread may post to the queue right after the wait() and it may return
	// true, but with a new pending task. Waiting is only real useful when you know
	// no one else is posting.
	// Will not wait if called on the execution thread because it might cause lock up!
	bool wait(int timeMs);

	bool wait() { return wait(exitWaitTimeMs_); }

private:
	void dropQueue();
	static QJniObject getMainThreadLooper();
	QJniObject createExecutor(const QJniObject & handler);

	static void jcallback(JNIEnv *, jobject, jlong ptr);
	void callback();

private:
	int exitWaitTimeMs_;
	QJniObject executor_;
	mutable QMutex taskQueueMutex_ { QMutex::Recursive };
	// Protected by taskQueueMutex_, locked if there are pending tasks
	// and unlocked otherwise.
	mutable QMutex hasPendingTasksMutex_ { QMutex::NonRecursive };
	// Protected by taskQueueMutex_
	using TaskQueue = std::queue<Task>;
	TaskQueue tasks_;
};
