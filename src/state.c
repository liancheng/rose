#include "detail/state.h"
#include "detail/sexp.h"

#include "rose/env.h"
#include "rose/port.h"
#include "rose/symbol.h"

#include <assert.h>
#include <gc/gc.h>
#include <string.h>

static void register_keyword (RState*     state,
                              ruint       index,
                              char const* symbol)
{
    state->keywords [index] = r_symbol_new_static (state, symbol);
}

static void register_keywords (RState* state)
{
    register_keyword (state, R_QUOTE,            "quote");
    register_keyword (state, R_LAMBDA,           "lambda");
    register_keyword (state, R_IF,               "if");
    register_keyword (state, R_SET_X,            "set!");
    register_keyword (state, R_BEGIN,            "begin");
    register_keyword (state, R_COND,             "cond");
    register_keyword (state, R_AND,              "and");
    register_keyword (state, R_OR,               "or");
    register_keyword (state, R_CASE,             "case");
    register_keyword (state, R_LET,              "let");
    register_keyword (state, R_LET_S,            "let*");
    register_keyword (state, R_LETREC,           "letrec");
    register_keyword (state, R_DO,               "do");
    register_keyword (state, R_DELAY,            "delay");
    register_keyword (state, R_QUASIQUOTE,       "quasiquote");
    register_keyword (state, R_ELSE,             "else");
    register_keyword (state, R_ARROW,            "=>");
    register_keyword (state, R_DEFINE,           "define");
    register_keyword (state, R_UNQUOTE,          "unquote");
    register_keyword (state, R_UNQUOTE_SPLICING, "unquote-splicing");
}

RState* r_state_new ()
{
    RState* state = GC_NEW (RState);

    memset (state, 0, sizeof (RState));

    state->symbol_table        = r_symbol_table_new ();
    state->env                 = r_env_new ();
    state->current_input_port  = r_stdin_port (state);
    state->current_output_port = r_stdout_port (state);
    state->error_jmp           = NULL;
    state->types               = GC_MALLOC_ATOMIC (sizeof (RType*) * 8);

    r_register_types (state);
    register_keywords (state);

    return state;
}

rsexp r_keyword (RState* state, ruint index)
{
    assert (index < R_KEYWORD_COUNT);
    return state->keywords [index];
}
