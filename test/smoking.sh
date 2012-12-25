#!/bin/sh

cd "`dirname $0`/.."
ROSE_ROOT=`pwd`

$ROSE_ROOT/bin/reader "$ROSE_ROOT/test/sample.scm" | diff - "$ROSE_ROOT/test/expected.scm"

if [ $? -eq 0 ]; then
    echo "smoking test passed"
else
    echo "smoking test failed"
fi
