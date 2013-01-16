%defines
%debug
%pure-parser
%error-verbose
%lex-param {quex_Calc_lexer  *qlex}
%parse-param {quex_Calc_lexer  *qlex}
%name-prefix="Calc_yy"

%{
#include "Calc_lexer.h"
#include <stdio.h>
#include <math.h>
#include <errno.h>
%}

%union
{
	double      dbl;
	const char* str;
};

%{
int  Calc_yylex(YYSTYPE *yylval, quex_Calc_lexer *qlex);
void Calc_yyerror(quex_Calc_lexer *qlex, const char* m);
%}

%type<dbl> exp num

%token<str> TKN_NUM
%left '-' '+'
%left '*' '/'
%left TKN_NEG     /* negation--unary minus */
%right '^'    /* exponentiation */

%%

%start input;

input:    /* empty */
        | input line
;

line:	'\n'
        | exp '\n'  { printf("\t%.10g\n", $1); }
;

exp:      num                { $$ = $1;         }
        | exp '+' exp        { $$ = $1 + $3;    }
        | exp '-' exp        { $$ = $1 - $3;    }
        | exp '*' exp        { $$ = $1 * $3;    }
        | exp '/' exp        { $$ = $1 / $3;    }
        | '-' exp  %prec TKN_NEG { $$ = -$2;        }
        | exp '^' exp        { $$ = pow ($1, $3); }
        | '(' exp ')'        { $$ = $2;         }
;

num:	TKN_NUM
{
	char *endptr;
	const char *str = $1;
	double val;
	val = strtod(str, &endptr);
	
	/* Check for various possible errors */
	if (errno == ERANGE || endptr == str)
	{
		Calc_yyerror(qlex, "strtod failed");
		YYABORT;
	}
	$$ = val;
	free((void*)$1);
}
        
%%

void Calc_yyerror(quex_Calc_lexer *qlex, const char*  m)
{
	printf("Parsing error at %i:%i: %s", 
           (int)qlex->counter._line_number_at_begin, (int)qlex->counter._column_number_at_begin, m);
           
}

int Calc_yylex(YYSTYPE *yylval, quex_Calc_lexer *qlex)
{
	QUEX_TYPE_TOKEN* token;

	QUEX_NAME(receive)(qlex, &token);

	if (strlen((char*)token->text) > 0 )
	{

		yylval->str = (char*)malloc((size_t)(strlen((char*)token->text)+1));
        memcpy((void*)yylval->str, (void*)token->text, (size_t)(strlen((char*)token->text) + 1));
	}
	return (int)token->_id;
}
