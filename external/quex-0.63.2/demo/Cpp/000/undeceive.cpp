       #include "EasyLexer"
       #include <stdio.h>
       #define __UNDECEIVE(X) "\n" #X "\n"
       #define UNDECEIVE(X)   __UNDECEIVE(X)
       int main(int argc, char** argv) { printf(UNDECEIVE(QUEX_NAME(receive))); }

