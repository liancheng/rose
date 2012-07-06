#include "detail/state.h"
#include "detail/sexp.h"

#include "rose/env.h"
#include "rose/port.h"
#include "rose/symbol.h"

#include <assert.h>
#include <gc/gc.h>
#include <string.h>

static void r_register_keyword (RState*     state,
                                ruint       index,
                                char const* symbol)
{
    state->keywords [index] = r_symbol_new_static (state, symbol);
}

static void r_register_keywords (RState* state)
{
    r_register_keyword (state, R_QUOTE,            "quote");
    r_register_keyword (state, R_LAMBDA,           "lambda");
    r_register_keyword (state, R_IF,               "if");
    r_register_keyword (state, R_SET_X,            "set!");
    r_register_keyword (state, R_BEGIN,            "begin");
    r_register_keyword (state, R_COND,             "cond");
    r_register_keyword (state, R_AND,              "and");
    r_register_keyword (state, R_OR,               "or");
    r_register_keyword (state, R_CASE,             "case");
    r_register_keyword (state, R_LET,              "let");
    r_register_keyword (state, R_LET_S,            "let*");
    r_register_keyword (state, R_LETREC,           "letrec");
    r_register_keyword (state, R_DO,               "do");
    r_register_keyword (state, R_DELAY,            "delay");
    r_register_keyword (state, R_QUASIQUOTE,       "quasiquote");
    r_register_keyword (state, R_ELSE,             "else");
    r_register_keyword (state, R_ARROW,            "=>");
    r_register_keyword (state, R_DEFINE,           "define");
    r_register_keyword (state, R_UNQUOTE,          "unquote");
    r_register_keyword (state, R_UNQUOTE_SPLICING, "unquote-splicing");
}

RState* r_state_new ()
{
    RState* state = GC_NEW (RState);

    memset (state, 0, sizeof (RState));

    state->symbol_table        = r_symbol_table_new ();
    state->env                 = r_env_new ();
    state->current_input_port  = r_stdin_port (state);
    state->current_output_port = r_stdout_port (state);

    r_register_tc3_types (state);
    r_register_keywords (state);

    return state;
}

rsexp r_keyword (RState* state, ruint index)
{
    assert (index < R_KEYWORD_COUNT);
    return state->keywords [index];
}
