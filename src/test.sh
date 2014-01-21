#!/bin/sh

if [ x$srcdir == "x" ]; then
  srcdir=.
fi

doline() {
  while read line; do
    v1=`echo $line|awk '{print $2;}'`
    v2=`echo $line|awk -F// '{print $2;}'|sed s/.txt//`
    if [ $v1 != $v2 ]; then
      echo bad line: $line
      exit 1
    else
      echo good: $line
    fi
  done
}


./sceadan_app $srcdir/../testdata/good/ 0  | doline

exit 0
