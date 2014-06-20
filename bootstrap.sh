#!/bin/sh
# Hopefully you checked out with git clone --recursive git@github.com:simsong/bulk_extractor.git

(cd liblinear;sh bootstrap.sh)
autoheader -f
touch NEWS README AUTHORS ChangeLog
touch stamp-h
aclocal -I m4
autoconf -f
libtoolize
automake --add-missing --copy
echo be sure to run ./configure
