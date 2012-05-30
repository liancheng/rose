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
#include "rose/vector.h"

#include "scanner.h"

#define KEYWORD(str) r_symbol_new_static ((str), state->context)

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

%type<sexp> abbreviation
%type<sexp> abbrev_prefix
%type<sexp> compound_datum
%type<sexp> data
%type<sexp> datum
%type<sexp> datum_seq
%type<sexp> improper_list
%type<sexp> list
%type<sexp> proper_list
%type<sexp> simple_datum
%type<sexp> start
%type<sexp> vector

%%

start
    : datum_seq                 { state->tree = $1; }
    ;

datum
    : simple_datum              { $$ = $1; }
    | compound_datum            { $$ = $1; }
    ;

data
    : datum                     { $$ = $1; }
    | data datum                { $$ = r_append_x ($1, r_list (1, $2)); }
    ;

datum_seq
    :                           { $$ = R_SEXP_NULL; }
    | data                      { $$ = $1; }
    ;

simple_datum
    : TKN_IDENTIFIER            { $$ = r_symbol_new ($1, state->context); }
    | TKN_STRING                { $$ = r_string_new ($1); }
    | TKN_BOOLEAN               { $$ = make_boolean ($1); }
    ;

compound_datum
    : list                      { $$ = $1; }
    | vector                    { $$ = $1; }
    ;

list
    : proper_list               { $$ = $1; }
    | improper_list             { $$ = $1; }
    | abbreviation              { $$ = $1; }
    ;

proper_list
    : "(" ")"                   { $$ = R_SEXP_NULL; }
    | "(" data ")"              { $$ = $2; }
    ;

improper_list
    : "(" data "." datum ")"    { $$ = r_append_x ($2, $4); }
    ;

abbreviation
    : abbrev_prefix datum       { $$ = r_list (2, $1, $2); }
    ;

abbrev_prefix
    : "'"                       { $$ = KEYWORD ("quote"); }
    | "`"                       { $$ = KEYWORD ("quasiquote");}
    | ","                       { $$ = KEYWORD ("unquote"); }
    | ",@"                      { $$ = KEYWORD ("unquote-splicing"); }
    ;

vector
    : "#(" datum_seq ")"        { $$ = r_list_to_vector ($2); }
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
