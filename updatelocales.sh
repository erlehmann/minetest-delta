#! /bin/bash
PROJECT_NAME=minetest-delta
xgettext -n -o $PROJECT_NAME.pot ./src/*.cpp ./src/*.h
msgmerge -U ./po/de/$PROJECT_NAME.po $PROJECT_NAME.pot
rm $PROJECT_NAME.pot
