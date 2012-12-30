#include "detail/gc.h"
#include "detail/port.h"
#include "detail/state.h"
#include "rose/gc.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

void init_bytevector_type_info    (RState* state);
void init_char_type_info          (RState* state);
void init_procedure_type_info     (RState* state);
void init_error_type_info         (RState* state);
void init_fixnum_type_info        (RState* state);
void init_flonum_type_info        (RState* state);
void init_native_type_info        (RState* state);
void init_opaque_type_info        (RState* state);
void init_pair_type_info          (RState* state);
void init_port_type_info          (RState* state);
void init_smi_type_info           (RState* state);
void init_special_const_type_info (RState* state);
void init_string_type_info        (RState* state);
void init_symbol_type_info        (RState* state);
void init_vector_type_info        (RState* state);

static void reserve (RState*       state,
                     ruint         index,
                     rconstcstring symbol)
{
    state->reserved [index] = r_symbol_new_static (state, symbol);
}

static void init_reserved_words (RState* state)
{
    reserve (state, KW_QUOTE,            "quote");
    reserve (state, KW_LAMBDA,           "lambda");
    reserve (state, KW_IF,               "if");
    reserve (state, KW_SET_X,            "set!");
    reserve (state, KW_BEGIN,            "begin");
    reserve (state, KW_COND,             "cond");
    reserve (state, KW_AND,              "and");
    reserve (state, KW_OR,               "or");
    reserve (state, KW_CASE,             "case");
    reserve (state, KW_LET,              "let");
    reserve (state, KW_LET_S,            "let*");
    reserve (state, KW_LETREC,           "letrec");
    reserve (state, KW_DO,               "do");
    reserve (state, KW_DELAY,            "delay");
    reserve (state, KW_QUASIQUOTE,       "quasiquote");
    reserve (state, KW_ELSE,             "else");
    reserve (state, KW_ARROW,            "=>");
    reserve (state, KW_DEFINE,           "define");
    reserve (state, KW_UNQUOTE,          "unquote");
    reserve (state, KW_UNQUOTE_SPLICING, "unquote-splicing");
    reserve (state, KW_CALL_CC,          "call/cc");

    reserve (state, INS_APPLY,           "apply");
    reserve (state, INS_ARG,             "arg");
    reserve (state, INS_ASSIGN,          "assign");
    reserve (state, INS_BRANCH,          "branch");
    reserve (state, INS_CAPTURE_CC,      "capture-cc");
    reserve (state, INS_CLOSE,           "close");
    reserve (state, INS_CONST,           "const");
    reserve (state, INS_BIND,            "bind");
    reserve (state, INS_FRAME,           "frame");
    reserve (state, INS_HALT,            "halt");
    reserve (state, INS_REFER,           "refer");
    reserve (state, INS_RESTORE_CC,      "restore-cc");
    reserve (state, INS_RETURN,          "return");
}

static void init_builtin_types (RState* state)
{
    /* Immediate types */
    init_char_type_info          (state);
    init_smi_type_info           (state);
    init_special_const_type_info (state);
    init_symbol_type_info        (state);

    /* Boxed types */
    init_pair_type_info          (state);
    init_bytevector_type_info    (state);
    init_error_type_info         (state);
    init_fixnum_type_info        (state);
    init_flonum_type_info        (state);
    init_native_type_info        (state);
    init_port_type_info          (state);
    init_string_type_info        (state);
    init_vector_type_info        (state);
    init_opaque_type_info        (state);
    init_procedure_type_info     (state);
}

static void init_std_ports (RState* state)
{
    state->current_input_port  = r_stdin_port  (state);
    state->current_output_port = r_stdout_port (state);
    state->current_error_port  = r_stderr_port (state);
}

void init_builtin_type (RState* state, RTypeTag tag, RTypeInfo* type)
{
    assert (R_TAG_RESERVED < tag && tag < R_TAG_MAX);
    state->builtin_types [tag] = *type;
}

rsexp reserved (RState* state, RReservedWord index)
{
    assert (index < RESERVED_WORD_COUNT);
    return state->reserved [index];
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

    gc_init (state);

    init_builtin_types  (state);
    init_std_ports      (state);
    init_reserved_words (state);

    vm_init (state);

exit:
    return state;
}

RState* r_state_open ()
{
    return r_state_new (default_alloc_fn, NULL);
}

void r_state_free (RState* state)
{
    vm_finish (state);
    gc_finish (state);
    r_free (state, state);
}
