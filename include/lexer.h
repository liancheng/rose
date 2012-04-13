#ifndef __ROSE_LEXER_H__
#define __ROSE_LEXER_H__

#include "r5rs_lexer.h"

typedef quex_Token          token;
typedef quex_r5rs_lexer     lexer;
typedef QUEX_TYPE_TOKEN_ID  token_id;

token*  read_token      (FILE* file);
lexer*  lexer_init      ();
void    lexer_finish    ();

#endif  //  __ROSE_LEXER_H__
