%defines
%name-prefix "rose_yy"
%define api.pure
%error-verbose

%lex-param   {RReaderState* state}
%parse-param {RReaderState* state}

%{

#include "detail/context.h"
#include "detail/reader.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/sexp.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#define YYSTYPE rsexp

#define KEYWORD(name) r_keyword ((name), state->context)

int  rose_yylex   (YYSTYPE*      yylval,
                   RReaderState* state);
void rose_yyerror (RReaderState* state,
                   char const*   message);

%}

%token TKN_FAIL

%token TKN_HASH_LP          "#("
%token TKN_HASH_SEMICOLON   "#;"
%token TKN_LP               "("
%token TKN_RP               ")"
%token TKN_DOT              "."
%token TKN_ELLIPSIS         "..."
%token TKN_COMMA_AT         ",@"
%token TKN_COMMA            ","
%token TKN_QUOTE            "'"
%token TKN_BACKTICK         "`"

%token TKN_IDENTIFIER
%token TKN_BOOLEAN
%token TKN_STRING
%token TKN_CHARACTER
%token TKN_NUMBER

%%

start
    :                           { state->tree = R_EOF; }
    | datum                     { state->tree = $1; YYACCEPT; }
    ;

datum
    : simple_datum              { $$ = $1; }
    | compound_datum            { $$ = $1; }
    ;

data
    : datum                     { $$ = r_list (1, $1); }
    | data datum                { $$ = r_append_x ($1, r_list (1, $2)); }
    ;

datum_seq
    :                           { $$ = R_NULL; }
    | data                      { $$ = $1; }
    ;

simple_datum
    : TKN_IDENTIFIER            { $$ = $1; }
    | TKN_STRING                { $$ = $1; }
    | TKN_BOOLEAN               { $$ = $1; }
    ;

compound_datum
    : list                      { $$ = $1; }
    | vector                    { $$ = $1; }
    ;

list
    : abbrev_prefix datum       { $$ = r_list (2, $1, $2); }
    | "(" datum_seq ")"         { $$ = $2; }
    | "(" data "." datum ")"    { $$ = r_append_x ($2, $4); }
    ;

abbrev_prefix
    : "'"                       { $$ = KEYWORD (R_QUOTE); }
    | "`"                       { $$ = KEYWORD (R_QUASIQUOTE); }
    | ","                       { $$ = KEYWORD (R_UNQUOTE); }
    | ",@"                      { $$ = KEYWORD (R_UNQUOTE_SPLICING); }
    ;

vector
    : "#(" datum_seq ")"        { $$ = r_list_to_vector ($2); }
    ;

%%

static rint reload_lexer (RLexer* lexer, rsexp port)
{
    QUEX_NAME (buffer_fill_region_prepare) (lexer);

    char* begin = (char*) QUEX_NAME (buffer_fill_region_begin) (lexer);
    rint  size  = QUEX_NAME (buffer_fill_region_size) (lexer);
    char* line  = r_port_gets (port, begin, size);
    rint  len   = line ? strlen (line) : 0;

    QUEX_NAME (buffer_fill_region_finish) (lexer, len);

    return len;
}

static RToken* copy_token (RToken* token)
{
    RToken* copy = malloc (sizeof (RToken));
    QUEX_NAME_TOKEN (copy_construct) (copy, token);
    return copy;
}

static void free_token (RToken* token)
{
    QUEX_NAME_TOKEN (destruct) (token);
    free (token);
}

static RToken* next_token (RLexer* lexer, rsexp port)
{
    RToken* token;

    QUEX_NAME (receive) (lexer, &token);

    while (TKN_TERMINATION == token->_id) {
        if (0 == reload_lexer (lexer, port))
            break;

        QUEX_NAME (receive) (lexer, &token);
    }

    return copy_token (token);
}

void rose_yyerror (RReaderState* state, char const* message)
{
    fprintf (stderr, "%s\n", message);
}

int rose_yylex (YYSTYPE* yylval, RReaderState* state)
{
    RToken* token;
    rtokenid id;
    char* text;

    token = next_token (state->lexer, state->input_port);
    id    = token->_id;
    text  = (char*) token->text;

    switch (id) {
        case TKN_IDENTIFIER:
            *yylval = r_symbol_new (text, state->context);
            break;

        case TKN_STRING:
            *yylval = r_string_new (text);
            break;

        case TKN_BOOLEAN:
            *yylval = (text [0] == 't') ? R_TRUE : R_FALSE;
            break;
    }

    free_token (token);

    return id;
}
