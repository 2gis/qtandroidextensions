#!/bin/bash

cd QtSources || exit 1
/bin/qtcreator animatedtiles.pro 2>&1 > /dev/null &
cd ..


