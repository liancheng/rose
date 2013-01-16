stamp=`date +%Yy%mm%dd-%Hh%M`
quick_f=false

case $1 in 
    HWUT-TEST) 
        output="tmp.dat"
        rm -f tmp.dat
        cd ..
        make clean >& /dev/null; 
        make lexer OPTIMIZATION=' ' EXTRA_COMPILER_FLAG='-DQUEX_QUICK_BENCHMARK_VERSION' >& /dev/null
        cd run
        ../lexer-quex linux-2.6.22.17-kernel-dir.c > $output
        make clean >& /dev/null; 
        exit
    ;;
    quick)
        quick_f=true
        output="tmp.dat"
        if [[ $2 != "" ]]; then
            quick_text=$2
        else
            quick_text=code/linux-2.6.22.17-kernel-dir.c
        fi
    ;;
    *)
        output="result-$stamp.dat"
    ;;
esac

function make_this {
    echo $1 $2
    cd ..
    make clean                  >& /dev/null 
    make run/$1 COMPILER_OPT="$2 -march=native" >& /dev/null
    cd run
}

function test_this {
    
    make_this $1 $2

    if [[ $quick_f == "false" ]]; then
        ./$1 code/many-tiny-tokens.c           >> $output
        ./$1 code/single-large-token.c         >> $output
        ./$1 code/linux-2.6.22.17-kernel-dir.c >> $output
    else
        ./$1 $quick_text | awk '/clock_cycles_per_token/ || /clock_cycles_per_character/'
    fi
}


echo "" > $output
if [[ $quick_f == "false" ]]; then
    test_this lexer-quex-lc -O3
fi
test_this lexer-quex        -O3

if [[ $quick_f == "false" ]]; then
    test_this lexer-quex-lc -Os
fi
test_this lexer-quex        -Os

