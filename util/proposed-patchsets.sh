#!/bin/sh

# Check what patches are proposed on the celeron55 wiki

# an auxiliary function to abort processing with an optional error
# message
abort() {
	test -n "$1" && echo >&2 "$1"
	exit 1
}

grabber=

# set grabber to some downloader
find_grabber()
{
	if type "curl" > /dev/null 2>&1 ; then
		grabber="curl -s"
	else if type "wget" > /dev/null 2>&1 ; then
		grabber="wget -q -O-"
	fi fi
}

find_grabber
test -z "$grabber" && abort "no grabber available"

url="http://celeron.55.lt/minetest/wiki/doku.php?id=patchsets&do=export_raw"

# echo celeron55's master branch first
echo "celeron55	git://github.com/celeron55/minetest.git	master"

# grab all the non-empty lines comprised between the 'For delta' section and the next one
# remove the leading 4 spaces and replace spaces with tabs

$grabber "$url" | sed \
	-e '1,/^=== For delta ===/d' -e'/^=/,$ d' -e'/^$/ d' \
	-e 's/    //' -e 's/ /\t/'
