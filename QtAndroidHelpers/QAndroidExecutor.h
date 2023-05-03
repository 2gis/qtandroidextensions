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
#include <atomic>
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

	// Check if the executor is in working state.
	bool isValid() const;

	// Returns true if current thread is the execution thread AND the executor is in valid state.
	bool isExecutionThread();

	// Post the task to the handler. Posted tasks will be executed asynchronously
	// in order of posting.
	// If the executor is in invald state, the task will not be posted.
	void post(Task && task);

	// Post the task to the handler, or, if we're already on the execution thread
	// execute it immediately.
	// If the executor is in invald state, the task will not be executed in either case.
	// Tasks that are executed from different threads may be executed out of order.
	void execute(Task && task);

	// Wait for empty queue for the specified time period (pass 0 to avoid waiting) and then
	// disable the executor so no more tasks can be added.
	// Tasks that were not executed are dropped.
	// Returns true is the queue was successfully emptied and no task has been lost.
	// Will not wait for tasks if called on the execution thread because it would cause lock up;
	// in that case all pending tasks get dropped.
	bool finish(int waitTimeMs);

	// Finish with timeout specified in the constructor.
	bool finish() { return finish(exitWaitTimeMs_); }

private:
	void dropQueue();
	bool wait(int waitTimeMs);
	static QJniObject getMainThreadLooper();
	QJniObject createExecutor(const QJniObject & handler);

	static void jcallback(JNIEnv *, jobject, jlong ptr);
	void callback();

private:
	int exitWaitTimeMs_;
	std::atomic<bool> finished_ { false };
	QJniObject executor_;
	mutable QMutex mainMutex_ { QMutex::Recursive };
	// Protected by mainMutex_, locked if there are pending tasks
	// and unlocked otherwise.
	mutable QMutex hasPendingTasksMutex_ { QMutex::NonRecursive };
	// Protected by mainMutex_
	using TaskQueue = std::queue<Task>;
	TaskQueue tasks_;
};
