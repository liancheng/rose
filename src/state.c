#include "detail/gc.h"
#include "detail/port.h"
#include "detail/state.h"
#include "rose/gc.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

void init_bytevector_type_info    (RState* r);
void init_char_type_info          (RState* r);
void init_procedure_type_info     (RState* r);
void init_error_type_info         (RState* r);
void init_fixnum_type_info        (RState* r);
void init_flonum_type_info        (RState* r);
void init_opaque_type_info        (RState* r);
void init_pair_type_info          (RState* r);
void init_port_type_info          (RState* r);
void init_primitive_type_info     (RState* r);
void init_smi_type_info           (RState* r);
void init_special_const_type_info (RState* r);
void init_string_type_info        (RState* r);
void init_symbol_type_info        (RState* r);
void init_vector_type_info        (RState* r);

static void reserve (RState*       r,
                     ruint         index,
                     rconstcstring symbol)
{
    r->reserved [index] = r_symbol_new_static (r, symbol);
}

static void state_init_reserved_words (RState* r)
{
    reserve (r, KW_QUOTE,            "quote");
    reserve (r, KW_LAMBDA,           "lambda");
    reserve (r, KW_IF,               "if");
    reserve (r, KW_SET_X,            "set!");
    reserve (r, KW_BEGIN,            "begin");
    reserve (r, KW_COND,             "cond");
    reserve (r, KW_AND,              "and");
    reserve (r, KW_OR,               "or");
    reserve (r, KW_CASE,             "case");
    reserve (r, KW_LET,              "let");
    reserve (r, KW_LET_S,            "let*");
    reserve (r, KW_LETREC,           "letrec");
    reserve (r, KW_DO,               "do");
    reserve (r, KW_DELAY,            "delay");
    reserve (r, KW_QUASIQUOTE,       "quasiquote");
    reserve (r, KW_ELSE,             "else");
    reserve (r, KW_ARROW,            "=>");
    reserve (r, KW_DEFINE,           "define");
    reserve (r, KW_UNQUOTE,          "unquote");
    reserve (r, KW_UNQUOTE_SPLICING, "unquote-splicing");
    reserve (r, KW_CALL_CC,          "call/cc");

    reserve (r, INS_APPLY,           "apply");
    reserve (r, INS_ARG,             "arg");
    reserve (r, INS_ASSIGN,          "assign");
    reserve (r, INS_BRANCH,          "branch");
    reserve (r, INS_CAPTURE_CC,      "capture-cc");
    reserve (r, INS_CLOSE,           "close");
    reserve (r, INS_CONST,           "const");
    reserve (r, INS_BIND,            "bind");
    reserve (r, INS_FRAME,           "frame");
    reserve (r, INS_HALT,            "halt");
    reserve (r, INS_REFER,           "refer");
    reserve (r, INS_RESTORE_CC,      "restore-cc");
    reserve (r, INS_RETURN,          "return");
}

static void state_init_builtin_types (RState* r)
{
    /* Immediate types */
    init_char_type_info          (r);
    init_smi_type_info           (r);
    init_special_const_type_info (r);
    init_symbol_type_info        (r);

    /* Boxed types */
    init_pair_type_info          (r);
    init_bytevector_type_info    (r);
    init_error_type_info         (r);
    init_fixnum_type_info        (r);
    init_flonum_type_info        (r);
    init_primitive_type_info     (r);
    init_port_type_info          (r);
    init_string_type_info        (r);
    init_vector_type_info        (r);
    init_opaque_type_info        (r);
    init_procedure_type_info     (r);
}

static void state_init_std_ports (RState* r)
{
    r->current_input_port  = r_stdin_port  (r);
    r->current_output_port = r_stdout_port (r);
    r->current_error_port  = r_stderr_port (r);
}

void init_builtin_type (RState* r, RTypeTag tag, RTypeInfo* type)
{
    assert (R_TAG_RESERVED < tag && tag < R_TAG_MAX);
    r->builtin_types [tag] = *type;
}

rsexp reserved (RState* r, RReservedWord index)
{
    assert (index < RESERVED_WORD_COUNT);
    return r->reserved [index];
}

RState* r_state_new (RAllocFunc alloc_fn, rpointer aux)
{
    RState* r;
    RState  zero = { { 0 } };

    r = alloc_fn (NULL, NULL, sizeof (RState));

    if (!r)
        goto exit;

    *r = zero;

    r->last_error = R_UNDEFINED;
    r->error_jmp  = NULL;
    r->alloc_fn   = alloc_fn;
    r->alloc_aux  = aux;

    gc_init (r);

    state_init_builtin_types  (r);
    state_init_std_ports      (r);
    state_init_reserved_words (r);

    vm_init (r);

exit:
    return r;
}

RState* r_state_open ()
{
    return r_state_new (default_alloc_fn, NULL);
}

void r_state_free (RState* r)
{
    vm_finish (r);
    gc_finish (r);

    r_free (r, r);
}
