/*
  Helper for QFile access to Android asset system for services

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The LGPL License

  Copyright (c) 2015, DoubleGIS, LLC.
  All rights reserved.

  GNU Lesser General Public License Usage
  This file is be used under the terms of the GNU Lesser
  General Public License version 2.1 or version 3 as published by the Free
  Software Foundation and appearing in the file LICENSE.LGPLv21 and
  LICENSE.LGPLv3 included in the packaging of this file. Please review the
  following information to ensure the GNU Lesser General Public License
  requirements will be met: https://www.gnu.org/licenses/lgpl.html and
  http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/

#pragma once
#include <QJniHelpers/QJniHelpers.h>

Q_DECL_IMPORT void installQAndroidAssetHandler(QJniObject & context);
