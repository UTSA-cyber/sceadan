#!/bin/sh

if [ x$srcdir == "x" ]; then
  srcdir=.
fi

./sceadan_app $srcdir/../testdata/SingleFilePerType/ 0

