#include "detail/sexp.h"
#include "detail/state.h"

#include "rose/env.h"
#include "rose/memory.h"
#include "rose/port.h"
#include "rose/state.h"
#include "rose/symbol.h"

#include <gc/gc.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

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

static rpointer default_alloc_fn (RState*  state,
                                  rpointer ptr,
                                  rsize    size,
                                  rpointer user_data)
{
    // Free the memory if size is 0.
    if (0 == size) {
        GC_FREE (ptr);
        return NULL;
    }

    if (NULL == ptr)
        return GC_MALLOC (size);

    return GC_REALLOC (ptr, size);
}

RState* r_state_new (RAllocFunc alloc_fn, rpointer user_data)
{
    RState* state = malloc (sizeof (RState));

    memset (state, 0, sizeof (RState));

    state->alloc_fn  = alloc_fn;
    state->user_data = user_data;

    state->symbol_table        = r_symbol_table_new (state);
    state->env                 = r_env_new (state);
    state->current_input_port  = r_stdin_port (state);
    state->current_output_port = r_stdout_port (state);
    state->error_jmp           = NULL;

    state->types = r_calloc (state, sizeof (RTypeDescriptor*), 8);

    r_register_types (state);
    register_keywords (state);

    return state;
}

RState* r_state_open ()
{
    return r_state_new (default_alloc_fn, NULL);
}

void r_state_free (RState* state)
{
    r_symbol_table_free (state, state->symbol_table);
    r_free (state, state->types);
    free (state);
}

rsexp r_keyword (RState* state, ruint index)
{
    assert (index < R_KEYWORD_COUNT);
    return state->keywords [index];
}

rchar* r_strdup (RState* state, rchar const* str)
{
    rsize  size;
    rchar* res;

    size = strlen (str);
    res = r_cast (rchar*, r_alloc (state, size + 1));
    memcpy (res, str, size + 1);

    return res;
}
