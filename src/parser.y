%defines
%name-prefix "rose_yy"
%define api.pure
%error-verbose

%lex-param   {RReaderState* state}
%parse-param {RReaderState* state}

%{

#include "detail/context.h"
#include "detail/reader.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/sexp.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#include <string.h>

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
    : TKN_BOOLEAN               { $$ = $1; }
    | TKN_NUMBER                { $$ = $1; }
    | TKN_CHARACTER             { $$ = $1; }
    | TKN_STRING                { $$ = $1; }
    | TKN_IDENTIFIER            { $$ = $1; }
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

static rsexp lexeme_to_char (char const* text)
{
    rint len = strlen (text);
    char res = *text;

    if (len > 1) {
        if      (0 == strcmp (text, "space"))     res = ' ';
        else if (0 == strcmp (text, "tab"))       res = '\t';
        else if (0 == strcmp (text, "newline"))   res = '\n';
        else if (0 == strcmp (text, "return"))    res = '\r';
        else if (0 == strcmp (text, "null"))      res = '\0';
        else if (0 == strcmp (text, "alarm"))     res = '\a';
        else if (0 == strcmp (text, "backspace")) res = '\b';
        else if (0 == strcmp (text, "escape"))    res = '\x1b';
        else if (0 == strcmp (text, "delete"))    res = '\x7f';
    }

    return r_char_to_sexp (res);
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

        case TKN_NUMBER:
            *yylval = r_string_to_number (text);
            break;

        case TKN_CHARACTER:
            *yylval = lexeme_to_char (text);
            break;
    }

    free_token (token);

    return id;
}
