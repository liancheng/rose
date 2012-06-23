#include "detail/context.h"
#include "detail/sexp.h"

#include "rose/env.h"
#include "rose/port.h"
#include "rose/symbol.h"

#include <assert.h>
#include <gc/gc.h>
#include <string.h>

static void r_register_keyword (ruint       index,
                                char const* symbol,
                                RContext*   context)
{
    context->keywords [index] = r_symbol_new_static (symbol, context);
}

static void r_register_keywords (RContext* context)
{
    r_register_keyword (R_QUOTE,            "quote",            context);
    r_register_keyword (R_LAMBDA,           "lambda",           context);
    r_register_keyword (R_IF,               "if",               context);
    r_register_keyword (R_SET_X,            "set!",             context);
    r_register_keyword (R_BEGIN,            "begin",            context);
    r_register_keyword (R_COND,             "cond",             context);
    r_register_keyword (R_AND,              "and",              context);
    r_register_keyword (R_OR,               "or",               context);
    r_register_keyword (R_CASE,             "case",             context);
    r_register_keyword (R_LET,              "let",              context);
    r_register_keyword (R_LET_S,            "let*",             context);
    r_register_keyword (R_LETREC,           "letrec",           context);
    r_register_keyword (R_DO,               "do",               context);
    r_register_keyword (R_DELAY,            "delay",            context);
    r_register_keyword (R_QUASIQUOTE,       "quasiquote",       context);
    r_register_keyword (R_ELSE,             "else",             context);
    r_register_keyword (R_ARROW,            "=>",               context);
    r_register_keyword (R_DEFINE,           "define",           context);
    r_register_keyword (R_UNQUOTE,          "unquote",          context);
    r_register_keyword (R_UNQUOTE_SPLICING, "unquote-splicing", context);
}

RContext* r_context_new ()
{
    RContext* context = GC_NEW (RContext);

    memset (context, 0, sizeof (RContext));

    context->symbol_table        = r_symbol_table_new ();
    context->env                 = r_env_new ();
    context->current_input_port  = r_stdin_port (context);
    context->current_output_port = r_stdout_port (context);

    r_register_tc3_types (context);
    r_register_keywords (context);

    return context;
}

rsexp r_keyword (ruint index, RContext* context)
{
    assert (index < R_KEYWORD_COUNT);

    return context->keywords [index];
}
