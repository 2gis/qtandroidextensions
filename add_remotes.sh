#!/bin/bash

git remote -v
echo "Adding remotes..."
git remote add gitlab git@gitlab.com:2gisqtandroid/qtandroidextensions.git
git remote add github git@github.com:2gis/qtandroidextensions.git


echo ""
echo "Current remotes:"
git remote -v
echo ""

git fetch --all
