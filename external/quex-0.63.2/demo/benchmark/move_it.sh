# This file is used by 'Makefile' to move quex lexical analyzer files.
# They shall still contain a lexer named 'quex_scan', but the file names
# shall be 'quex_$1_scan' where '$q' is the first argument passed to this script.
for file in `ls out/quex_scan*`; do mv $file `echo $file | sed -e "s/quex_scan/quex_$1_scan/g"`; done
