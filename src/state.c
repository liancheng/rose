#include "detail/port.h"
#include "detail/state.h"
#include "rose/gc.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

void init_bool_type_info          (RState* state);
void init_bytevector_type_info    (RState* state);
void init_char_type_info          (RState* state);
void init_error_type_info         (RState* state);
void init_fixnum_type_info        (RState* state);
void init_flonum_type_info        (RState* state);
void init_pair_type_info          (RState* state);
void init_port_type_info          (RState* state);
void init_smi_type_info           (RState* state);
void init_special_const_type_info (RState* state);
void init_string_type_info        (RState* state);
void init_symbol_type_info        (RState* state);
void init_vector_type_info        (RState* state);

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
    RState  zero = { 0 };
    RState* state;

    state = alloc_fn (NULL, NULL, sizeof (RState));

    if (!state)
        goto exit;

    *state = zero;

    state->gc_list    = NULL;
    state->last_error = R_UNDEFINED;
    state->error_jmp  = NULL;
    state->alloc_fn   = alloc_fn;
    state->alloc_aux  = aux;

    init_builtin_types (state);
    init_std_ports (state);
    init_keywords (state);

exit:
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

void r_state_free (RState* state)
{
    free_builtin_types (state);
    r_free (state, state);
}
