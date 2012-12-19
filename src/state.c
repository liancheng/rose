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

static void init_gc (RState* state)
{
    gc_state_init (state, &state->gc);
}

RState* r_state_new (RAllocFunc alloc_fn, rpointer aux)
{
    RState* state;
    RState  zero = { { 0 } };

    state = alloc_fn (NULL, NULL, sizeof (RState));

    if (!state)
        goto exit;

    *state = zero;

    state->last_error = R_UNDEFINED;
    state->error_jmp  = NULL;
    state->alloc_fn   = alloc_fn;
    state->alloc_aux  = aux;

    init_gc (state);

    init_builtin_types (state);
    init_std_ports     (state);
    init_keywords      (state);

exit:
    return state;
}

RState* r_state_open ()
{
    return r_state_new (default_alloc_fn, NULL);
}

static void free_builtin_types (RState* state)
{
    r_free (state, state->builtin_types [R_CHAR_TAG]);
    r_free (state, state->builtin_types [R_SMI_TAG]);
    r_free (state, state->builtin_types [R_SPECIAL_CONST_TAG]);
    r_free (state, state->builtin_types [R_SYMBOL_TAG]);
    r_free (state, state->builtin_types [R_PAIR_TAG]);
    r_free (state, state->builtin_types [R_STRING_TAG]);
    r_free (state, state->builtin_types [R_VECTOR_TAG]);
    r_free (state, state->builtin_types [R_BYTEVECTOR_TAG]);
    r_free (state, state->builtin_types [R_PORT_TAG]);
    r_free (state, state->builtin_types [R_ERROR_TAG]);
    r_free (state, state->builtin_types [R_FIXNUM_TAG]);
    r_free (state, state->builtin_types [R_FLONUM_TAG]);
}

rsexp keyword (RState* state, ruint index)
{
    assert (index < R_KEYWORD_COUNT);
    return state->keywords [index];
}

void r_state_free (RState* state)
{
    gc_scope_reset (state);
    r_full_gc (state);
    gc_state_destruct (state, &state->gc);
    free_builtin_types (state);
    r_free (state, state);
}
