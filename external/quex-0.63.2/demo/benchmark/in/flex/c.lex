/* Scanner for 'C' 
 * (C) 2008 Frank-Rene Schaefer
 * LICENSE: GPL */
%{
#include "in/token-ids.h"
%}

P_WHITESPACE          [ \t\n]+
P_IDENTIFIER          [_a-zA-Z][_a-zA-Z0-9]*
P_NUMBER              [0-9]+
P_STRING              "\""(\\"\""|[^"])*"\""
P_QUOTED_CHAR_1       ("'\\''")|("'"[^']?"'")
P_QUOTED_CHAR_2       "'\\"[0-9abcfnrtv\\]"'"
P_QUOTED_CHAR         ({P_QUOTED_CHAR_1}|{P_QUOTED_CHAR_2})
P_INCLUDE_FILE1       "<"[^>]+">"
P_INCLUDE_FILE2       "\""[^"]+"\""

%%
{P_WHITESPACE}                                 { }
"/*"([^*]|[\r\n]|("*"+([^*/]|[\r\n])))*"*"+"/" { }
"//"[^\n]*"\n"                                 { } 

"#"[ \t]*"include"[ \t]*{P_INCLUDE_FILE2} return TKN_PP_INCLUDE;
"#"[ \t]*"include"[ \t]*{P_INCLUDE_FILE1} return TKN_PP_INCLUDE;

"#"[ \t]*"define"  return TKN_PP_DEFINE;
"#"[ \t]*"if"      return TKN_PP_IF;
"#"[ \t]*"elif"    return TKN_PP_ELIF;
"#"[ \t]*"ifdef"   return TKN_PP_IFDEF;
"#"[ \t]*"ifndef"  return TKN_PP_IFNDEF;
"#"[ \t]*"endif"   return TKN_PP_ENDIF;
"#"[ \t]*"else"    return TKN_PP_ELSE;
"#"[ \t]*"pragma"  return TKN_PP_PRAGMA;
"#"[ \t]*"error"   return TKN_PP_ERROR;
defined            return TKN_PP_DEFINED;
"\\\n"             return TKN_BACKLASHED_NEWLINE;

unsigned  return TKN_TYPE_UNSIGNED;
int       return TKN_TYPE_INT;
long      return TKN_TYPE_LONG;
float     return TKN_TYPE_FLOAT;
double    return TKN_TYPE_DOUBLE;
char      return TKN_TYPE_CHAR;

"#"           return TKN_HASH;
"##"          return TKN_DOUBLE_HASH;
"?"           return TKN_QUESTION_MARK;
"~"           return TKN_TILDE;
"("           return TKN_BRACKET_O;
")"           return TKN_BRACKET_C;
"["           return TKN_CORNER_BRACKET_O;
"]"           return TKN_CORNER_BRACKET_C;
"{"           return TKN_CURLY_BRACKET_O;
"}"           return TKN_CURLY_BRACKET_C;
"="           return TKN_OP_ASSIGNMENT;
"+"           return TKN_PLUS;
"-"           return TKN_MINUS;
"*"           return TKN_MULT;
"/"           return TKN_DIV;
"%"           return TKN_MODULO;
"+="          return TKN_ASSIGN_PLUS;
"-="          return TKN_ASSIGN_MINUS;
"*="          return TKN_ASSIGN_MULT;
"/="          return TKN_ASSIGN_DIV;
"=="          return TKN_EQ;
"!="          return TKN_NOT_EQ;
">"           return TKN_GREATER;
">="          return TKN_GR_EQ;
"<"           return TKN_LESS;
"<="          return TKN_LE_EQ;
"!"           return TKN_NOT;
"|"           return TKN_LOGICAL_OR;
"^"           return TKN_EXCLUSIVE_OR;
"||"          return TKN_OR;
"&"           return TKN_AMPERSANT;
"&&"          return TKN_AND;
":"           return TKN_COLON;
struct        return TKN_STRUCT;
const         return TKN_CONST;
if            return TKN_IF;
else          return TKN_ELSE;
switch        return TKN_SWITCH;
for           return TKN_FOR;
do            return TKN_DO;
while         return TKN_WHILE;
break         return TKN_BREAK;
continue      return TKN_CONTINUE;
";"           return TKN_SEMICOLON;
"."           return TKN_DOT;
","           return TKN_COMMA;

{P_IDENTIFIER}  return TKN_IDENTIFIER;
{P_NUMBER}      return TKN_NUMBER;
{P_STRING}      return TKN_STRING;
{P_QUOTED_CHAR} return TKN_QUOTED_CHAR;

%%
