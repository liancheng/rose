#ifndef __QUEX_INCLUDE_GUARD__BENCHMARK__RE2C__ADAPTION_H
#define __QUEX_INCLUDE_GUARD__BENCHMARK__RE2C__ADAPTION_H

extern char*  global_re2c_buffer_begin;
extern char*  global_re2c_buffer_end;
extern char*  global_re2c_buffer_iterator;

#include "in/token-ids.h"
#define  TKN_TERMINATION (0)

#define QUEX_TYPE_TOKEN_ID  int
 QUEX_TYPE_TOKEN_ID re2c_scan(char** p);
#define ANALYZER_ANALYZE(TokenID) \
        do {                                                   \
            TokenID = re2c_scan(&global_re2c_buffer_iterator); \
        } while ( 0 )

#define ANALYZER_RESET() \
        do {                                                        \
            global_re2c_buffer_iterator = global_re2c_buffer_begin; \
        } while ( 0 )


#endif
