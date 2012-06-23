#!/bin/sh

cd `dirname $0`
cd ..
ROOT=`pwd`

$ROOT/bin/rsi $ROOT/test/test.scm | diff - $ROOT/test/expected.scm
