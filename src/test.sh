#!/bin/sh

if [ x$srcdir == "x" ]; then
  srcdir=.
fi

bads=no

doline() {
  while read line; do
    expected=`echo $line|sed 's:.*/::' | sed s/.txt// | tr '[a-z]' '[A-Z]'`
    classified=`echo $line|awk '{print $2;}'`
    if [ $expected != $classified ]; then
      echo expected $expected got $classified line: $line
      bads=yes
    else
      echo good: $line
    fi
  done
}


echo Good data here:
./sceadan_app $srcdir/../data_test/good   | doline

if [ $bads != "no" ]; then
  exit 1;
fi  

exit 0
