#ifndef __QUEX_INCLUDE_GUARD__BENCHMARK__QUEX__ADAPTION_H
#define __QUEX_INCLUDE_GUARD__BENCHMARK__QUEX__ADAPTION_H

#include "out/quex_scan"

extern quex::quex_scan*  global_qlex; 
extern quex::Token       global_token; 

using namespace quex;

#ifdef QUEX_OPTION_TOKEN_POLICY_SINGLE
#   define ANALYZER_ANALYZE(TokenID)       \
            do {                           \
                TokenID = global_qlex->receive(); \
            } while( 0 )
#else
#   define ANALYZER_ANALYZE(TokenID)       \
            do {                           \
                global_qlex->receive(&global_token); \
                TokenID = global_token.type_id();    \
            } while( 0 )
#endif
#define ANALYZER_RESET() \
            do {                          \
                fseek(global_fh, 100, SEEK_SET); \
                global_qlex->reset(global_fh);   \
            } while( 0 )

#endif /* __QUEX_INCLUDE_GUARD__BENCHMARK__QUEX__ADAPTION_H */
