#include "detail/bytevector.h"
#include "detail/error.h"
#include "detail/number.h"
#include "detail/pair.h"
#include "detail/port.h"
#include "detail/sexp.h"
#include "detail/state.h"
#include "detail/string.h"
#include "detail/symbol.h"
#include "detail/vector.h"

#include "rose/memory.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

static void init_keyword (RState*       state,
                          ruint         index,
                          rconstcstring symbol)
{
    state->keywords [index] = r_symbol_new_static (state, symbol);
}

static void init_keywords (RState* state)
{
    init_keyword (state, R_QUOTE,            "quote");
    init_keyword (state, R_LAMBDA,           "lambda");
    init_keyword (state, R_IF,               "if");
    init_keyword (state, R_SET_X,            "set!");
    init_keyword (state, R_BEGIN,            "begin");
    init_keyword (state, R_COND,             "cond");
    init_keyword (state, R_AND,              "and");
    init_keyword (state, R_OR,               "or");
    init_keyword (state, R_CASE,             "case");
    init_keyword (state, R_LET,              "let");
    init_keyword (state, R_LET_S,            "let*");
    init_keyword (state, R_LETREC,           "letrec");
    init_keyword (state, R_DO,               "do");
    init_keyword (state, R_DELAY,            "delay");
    init_keyword (state, R_QUASIQUOTE,       "quasiquote");
    init_keyword (state, R_ELSE,             "else");
    init_keyword (state, R_ARROW,            "=>");
    init_keyword (state, R_DEFINE,           "define");
    init_keyword (state, R_UNQUOTE,          "unquote");
    init_keyword (state, R_UNQUOTE_SPLICING, "unquote-splicing");
}

static void init_builtin_types (RState* state)
{
    /* Immediate types */
    init_char_type_info          (state);
    init_smi_type_info           (state);
    init_special_const_type_info (state);
    init_symbol_type_info        (state);

    /* Non-immediate types */
    init_pair_type_info          (state);
    init_bytevector_type_info    (state);
    init_error_type_info         (state);
    init_fixnum_type_info        (state);
    init_flonum_type_info        (state);
    init_port_type_info          (state);
    init_string_type_info        (state);
    init_vector_type_info        (state);
}

static void init_std_ports (RState* state)
{
    state->current_input_port  = r_stdin_port  (state);
    state->current_output_port = r_stdout_port (state);
    state->current_error_port  = r_stderr_port (state);
}

RState* r_state_new (RAllocFunc alloc_fn, rpointer aux)
{
    RState* state = alloc_fn (NULL, NULL, sizeof (RState));

    if (state)
        memset (state, 0, sizeof (RState));
    else
        return NULL;

    state->gc_list    = NULL;
    state->last_error = R_UNDEFINED;
    state->error_jmp  = NULL;
    state->alloc_fn   = alloc_fn;
    state->alloc_aux  = aux;

    init_builtin_types (state);
    init_std_ports (state);
    init_keywords (state);

    return state;
}

RState* r_state_open ()
{
    return r_state_new (default_alloc_fn, NULL);
}

static void free_builtin_types (RState* state)
{
    rsize i;

    for (i = 0; i < R_TAG_MAX; ++i)
        if (state->builtin_types [i])
            r_free (state, state->builtin_types [i]);
}

void r_state_free (RState* state)
{
    free_builtin_types (state);
    r_free (state, state);
}

rsexp keyword (RState* state, ruint index)
{
    assert (index < R_KEYWORD_COUNT);
    return state->keywords [index];
}

rcstring cstring_dup (RState* state, rconstcstring str)
{
    rsize    size = strlen (str);
    rcstring res  = r_cast (rcstring, r_new0_array (state, rchar, size + 1));

    if (res)
        memcpy (res, str, size + 1);

    return res;
}

rsexp r_last_error (RState* state)
{
    return state->last_error;
}

void r_set_last_error_x (RState* state, rsexp error)
{
    state->last_error = error;
}

void r_inherit_errno_x (RState* state, rint errnum)
{
    char buffer [BUFSIZ];
    strerror_r (errnum, buffer, BUFSIZ);
    r_set_last_error_x (state,
                        r_error_new (state,
                                     r_string_new (state, buffer),
                                     R_NULL));
}
