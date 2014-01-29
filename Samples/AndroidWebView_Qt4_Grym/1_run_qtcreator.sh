#!/bin/bash

cd QtSources || exit 1
/bin/qtcreator \
  ../../../QJniHelpers/QJniHelpers.pro \
  ../../../QtOffscreenViews/QtOffscreenViews.pro \
  androidwebview.pro  2>&1 > /dev/null &
cd ..


