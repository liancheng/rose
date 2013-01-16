/* -*- C++ -*- vim: set syntax=cpp:
 * PURPOSE: File containing definition of token-identifier and
 *          a function that maps token identifiers to a string
 *          name.
 *
 * NOTE: This file has been created automatically by Quex.
 *       Visit quex.org for further info.
 *
 * DATE: Tue Jun 26 22:10:19 2012
 *
 * (C) 2005-2010 Frank-Rene Schaefer
 * ABSOLUTELY NO WARRANTY                                           */
#ifndef __QUEX_INCLUDE_GUARD__AUTO_TOKEN_IDS_QUEX_UTF16LEX__QUEX_TOKEN__
#define __QUEX_INCLUDE_GUARD__AUTO_TOKEN_IDS_QUEX_UTF16LEX__QUEX_TOKEN__

#ifndef __QUEX_OPTION_PLAIN_C
#   include<cstdio> 
#else
#   include<stdio.h> 
#endif

/* The token class definition file can only be included after 
 * the definition on TERMINATION and UNINITIALIZED.          
 * (fschaef 12y03m24d: "I do not rememember why I wrote this.
 *  Just leave it there until I am clear if it can be deleted.")   */
#include "UTF16Lex-token.h"

#define TKN_COLON         ((QUEX_TYPE_TOKEN_ID)10000)
#define TKN_DEDENT        ((QUEX_TYPE_TOKEN_ID)3)
#define TKN_INDENT        ((QUEX_TYPE_TOKEN_ID)2)
#define TKN_NODENT        ((QUEX_TYPE_TOKEN_ID)4)
#define TKN_NUMBER        ((QUEX_TYPE_TOKEN_ID)10001)
#define TKN_TERMINATION   ((QUEX_TYPE_TOKEN_ID)0)
#define TKN_UNINITIALIZED ((QUEX_TYPE_TOKEN_ID)1)
#define TKN_UNKNOWN       ((QUEX_TYPE_TOKEN_ID)10002)
#define TKN_WHITE         ((QUEX_TYPE_TOKEN_ID)10003)
#define TKN_WORD          ((QUEX_TYPE_TOKEN_ID)10004)


QUEX_NAMESPACE_TOKEN_OPEN
extern const char* QUEX_NAME_TOKEN(map_id_to_name)(const QUEX_TYPE_TOKEN_ID TokenID);
QUEX_NAMESPACE_TOKEN_CLOSE

#endif /* __QUEX_INCLUDE_GUARD__AUTO_TOKEN_IDS_QUEX_UTF16LEX__QUEX_TOKEN__ */
