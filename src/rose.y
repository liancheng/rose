%defines
%name-prefix "rose_yy"
%define api.pure
%error-verbose

%lex-param   {RReaderState* state}
%parse-param {RReaderState* state}

%{

#include "rose/pair.h"
#include "rose/read.h"
#include "rose/sexp.h"
#include "rose/string.h"
#include "rose/symbol.h"

#include "scanner.h"

int  rose_yylex   (YYSTYPE*      yylval,
                   RReaderState* scanner);
void rose_yyerror (RReaderState* scanner,
                   char const*   message);

rsexp make_boolean (char* text)
{
    return text [0] == 't' ? R_SEXP_TRUE : R_SEXP_FALSE;
}

%}

%union {
    char* text;
    rsexp sexp;
}

%token TKN_EXPECT_MORE
%token TKN_EOF
%token TKN_FAIL

%token TKN_HASH_LP          "#("
%token TKN_HASH_U8_LP       "#u8("
%token TKN_HASH_SEMICOLON   "#;"
%token TKN_LP               "("
%token TKN_RP               ")"
%token TKN_DOT              "."
%token TKN_ELLIPSIS         "..."
%token TKN_COMMA_AT         ",@"
%token TKN_COMMA            ","
%token TKN_QUOTE            "'"
%token TKN_BACKTICK         "`"

%token<text> TKN_IDENTIFIER
%token<text> TKN_BOOLEAN
%token<text> TKN_STRING
%token<text> TKN_CHARACTER

%type<sexp> start
%type<sexp> datum
%type<sexp> datum_seq
%type<sexp> data
%type<sexp> simple_datum

%%

start
    : datum_seq             { state->tree = $1; }
    ;

datum
    : simple_datum          { $$ = $1; }
    ;

data
    : datum                 { $$ = $1; }
    | data datum            { $$ = r_append_x ($1, r_list (1, $2)); }
    ;

datum_seq
    :                       { $$ = R_SEXP_NULL; }
    | data                  { $$ = $1; }
    ;

simple_datum
    : TKN_IDENTIFIER        { $$ = r_symbol_new ($1, state->context); }
    | TKN_STRING            { $$ = r_string_new ($1); }
    | TKN_BOOLEAN           { $$ = make_boolean ($1); }
    ;

%%

void rose_yyerror (RReaderState* state, char const* message)
{
    fprintf (stderr, "%s\n", message);
}

int rose_yylex (YYSTYPE* yylval, RReaderState* state)
{
    RToken* t;

    t = r_scanner_next_token (state->scanner, state->input_port);
    yylval->text = (char*) t->text;

    return t->_id;
}
