for x in `find -maxdepth 3 -mindepth 1 -type f -or -path "*.svn*" -prune -or -print`; do 
    echo "###### $x"; 
    cd $x; 
    make clean; 
    rm -f tmp.txt
    rm -f tmp[10].txt
    cd $QUEX_PATH/demo; 
done
