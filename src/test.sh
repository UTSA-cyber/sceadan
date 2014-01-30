#!/bin/sh

if [ x$srcdir == "x" ]; then
  srcdir=.
fi

bads=no

doline() {
  while read line; do
    v1=`echo $line|awk '{print $2;}'`
    v2=`echo $line|sed 's:.*/::' | sed s/.txt//`
    if [ $v1 != $v2 ]; then
      echo bad line: $line
      bads=yes
    else
      echo good: $line
    fi
  done
}


./sceadan_app $srcdir/../testdata/good   | doline

if [ $bads != "no" ]; then
  exit 1;
fi  

exit 0
