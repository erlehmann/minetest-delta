#!/bin/sh
#find src -name '*.cpp' -o -name '*.h' -exec astyle -S -a -k3 -t8 -H -U -p -S -K -O '{}' ';' | grep -v 'unchanged'
find src -name '*.cpp' , -name '*.h' -exec astyle --style=ansi --indent=tab '{}' ';' | grep -v 'unchanged'
find src -name '*.cpp' -exec astyle --style=ansi --indent=tab '{}' ';' | grep -v 'unchanged'
