/****************************************************************************
**
** Copyright (C) 2012 BogDan Vatra <bogdan@kde.org>
** Contact: http://www.qt-project.org/legal
**
** GNU Lesser General Public License Usage
** This file is be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
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

