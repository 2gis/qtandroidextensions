/*
  Offscreen Android Views library for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2014, DoubleGIS, LLC.
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

#include <QCoreApplication>
#include <QDebug>
#include "QApplicationActivityObserver.h"


QApplicationActivityObserver * QApplicationActivityObserver::instance()
{
	static QMutex mymutex;
	static QScopedPointer<QApplicationActivityObserver> instance;
	QMutexLocker locker(&mymutex);
	if (instance.isNull())
	{
		instance.reset(new QApplicationActivityObserver());
	}
	return instance.data();
}

void QApplicationActivityObserver::setApplicationActive(bool active)
{
	if (active != is_active_)
	{
		is_active_ = active;
		emit applicationActiveStateChanged();
	}
}

void QApplicationActivityObserver::installQApplicationEventFilter()
{
	QCoreApplication * app = QCoreApplication::instance();
	if (app)
	{
		app->installEventFilter(instance());
	}
	else
	{
		qWarning()<<__FUNCTION__<<"failed because QApplication instance doesn't exist.";
	}
}

bool QApplicationActivityObserver::eventFilter(QObject * obj, QEvent * event)
{
	if (event->type() == QEvent::ApplicationActivate)
	{
		qDebug()<<__PRETTY_FUNCTION__<<"ACTIVE!";
		QMetaObject::invokeMethod(this, "setApplicationActive", Qt::QueuedConnection, Q_ARG(bool,true));
	}
	else if (event->type() == QEvent::ApplicationDeactivate)
	{
		qDebug()<<__PRETTY_FUNCTION__<<"DEACTIVATE!";
		QMetaObject::invokeMethod(this, "setApplicationActive", Qt::QueuedConnection, Q_ARG(bool,false));
	}
	return QObject::eventFilter(obj, event);
}

