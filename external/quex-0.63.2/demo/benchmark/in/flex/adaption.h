#ifndef __QUEX_INCLUDE_GUARD__BENCHMARK__RE2C__ADAPTION_H
#define __QUEX_INCLUDE_GUARD__BENCHMARK__RE2C__ADAPTION_H

extern int    yylex();
extern FILE*  yyin;
extern void   yyrestart(FILE*);

typedef int QUEX_TYPE_TOKEN_ID;

#define ANALYZER_ANALYZE(TokenID) \
        do {                      \
            TokenID = yylex();    \
        } while ( 0 )

#define ANALYZER_RESET() \
        do {                          \
           fseek(yyin, 0, SEEK_SET);  \
           yyrestart(yyin);           \
        } while( 0 )

#endif
