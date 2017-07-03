/****************************************************************************
**
** Copyright (C) 2012 BogDan Vatra <bogdan@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma once

#include <QtCore/qtcoreversion.h>

#if QTCORE_VERSION == 0x050401
#   include <QtCore/5.4.1/QtCore/private/qabstractfileengine_p.h>
#elif QTCORE_VERSION == 0x050500
#   include <QtCore/5.5.0/QtCore/private/qabstractfileengine_p.h>
#elif QTCORE_VERSION == 0x050501
#   include <QtCore/5.5.1/QtCore/private/qabstractfileengine_p.h>
#elif QTCORE_VERSION == 0x050600
#   include <QtCore/5.6.0/QtCore/private/qabstractfileengine_p.h>
#elif QTCORE_VERSION == 0x050601
#   include <QtCore/5.6.1/QtCore/private/qabstractfileengine_p.h>
#elif QTCORE_VERSION == 0x050602
#   include <QtCore/5.6.2/QtCore/private/qabstractfileengine_p.h>
#elif QTCORE_VERSION == 0x050800
#   include <QtCore/5.8.0/QtCore/private/qabstractfileengine_p.h>
#elif QTCORE_VERSION == 0x050900
#   include <QtCore/5.9.0/QtCore/private/qabstractfileengine_p.h>
#else
#   error qt version unknown
#endif

#include <QtCore/QCache>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <android/asset_manager.h>

struct AndroidAssetDir;

class AndroidAssetsFileEngineHandler: public QObject, public QAbstractFileEngineHandler
{
    Q_OBJECT
public:
    AndroidAssetsFileEngineHandler(AAssetManager * assetManager);
    virtual ~AndroidAssetsFileEngineHandler();
    QAbstractFileEngine *create(const QString &fileName) const;

private:
    void prepopulateCache() const;

    AAssetManager *m_assetManager;
    mutable QCache<QByteArray, QSharedPointer<AndroidAssetDir>> m_assetsCache;
    mutable QMutex m_assetsCacheMutext;
    mutable bool m_hasPrepopulatedCache;
    mutable bool m_hasTriedPrepopulatingCache;
};

