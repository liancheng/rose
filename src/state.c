#include "detail/gc.h"
#include "detail/io.h"
#include "detail/state.h"
#include "rose/gc.h"
#include "rose/number.h"
#include "rose/string.h"
#include "rose/symbol.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

extern RTypeInfo char_type;
extern RTypeInfo small_int_type;
extern RTypeInfo special_const_type;
extern RTypeInfo symbol_type;

extern RTypeInfo bytevector_type;
extern RTypeInfo error_type;
extern RTypeInfo opaque_type;
extern RTypeInfo pair_type;
extern RTypeInfo port_type;
extern RTypeInfo primitive_type;
extern RTypeInfo procedure_type;
extern RTypeInfo string_type;
extern RTypeInfo vector_type;

extern RTypeInfo fixreal_type;
extern RTypeInfo floreal_type;
extern RTypeInfo complex_type;

static void init_reserved_words (RState* r)
{
    r_intern (r, "quote");
    r_intern (r, "lambda");
    r_intern (r, "if");
    r_intern (r, "set!");
    r_intern (r, "quasiquote");
    r_intern (r, "define");
    r_intern (r, "unquote");
    r_intern (r, "unquote-splicing");
    r_intern (r, "call/cc");

    r_intern (r, "apply");
    r_intern (r, "arg");
    r_intern (r, "assign");
    r_intern (r, "branch");
    r_intern (r, "capture-cc");
    r_intern (r, "close");
    r_intern (r, "constant");
    r_intern (r, "bind");
    r_intern (r, "frame");
    r_intern (r, "halt");
    r_intern (r, "refer");
    r_intern (r, "restore-cc");
    r_intern (r, "return");
}

static void init_oom_error (RState* r)
{
    rsexp message = r_string_new (r, "out of memory");
    r->oom_error = r_error_new (r, message, R_NULL);
}

static void init_const_number (RState* r)
{
    r->flo_zero = r_floreal_new (r, 0.);
    r->flo_one = r_floreal_new (r, 1.);
}

static void init_builtin_types (RState* r)
{
    /* Immediate types */
    r->builtin_types [R_TAG_CHAR]          = char_type;
    r->builtin_types [R_TAG_SMI_EVEN]      = small_int_type;
    r->builtin_types [R_TAG_SMI_ODD]       = small_int_type;
    r->builtin_types [R_TAG_SPECIAL_CONST] = special_const_type;
    r->builtin_types [R_TAG_SYMBOL]        = symbol_type;

    /* Boxed types */
    r->builtin_types [R_TAG_BYTEVECTOR]    = bytevector_type;
    r->builtin_types [R_TAG_ERROR]         = error_type;
    r->builtin_types [R_TAG_OPAQUE]        = opaque_type;
    r->builtin_types [R_TAG_PAIR]          = pair_type;
    r->builtin_types [R_TAG_PORT]          = port_type;
    r->builtin_types [R_TAG_PRIMITIVE]     = primitive_type;
    r->builtin_types [R_TAG_PROCEDURE]     = procedure_type;
    r->builtin_types [R_TAG_STRING]        = string_type;
    r->builtin_types [R_TAG_VECTOR]        = vector_type;
    r->builtin_types [R_TAG_FIXREAL]       = fixreal_type;
    r->builtin_types [R_TAG_FLOREAL]       = floreal_type;
    r->builtin_types [R_TAG_FIX_COMPLEX]   = complex_type;
    r->builtin_types [R_TAG_FLO_COMPLEX]   = complex_type;
}

static void init_std_ports (RState* r)
{
    r->current_input_port  = r_stdin_port  (r);
    r->current_output_port = r_stdout_port (r);
    r->current_error_port  = r_stderr_port (r);
}

static void init_global_objects (RState* r)
{
    init_oom_error (r);
    init_const_number (r);
    init_reserved_words (r);
    init_std_ports (r);
}

static rpointer default_alloc_fn (rpointer aux, rpointer ptr, rsize size)
{
    assert (!(ptr == 0 && size == 0u));

    if (0 == size) {
        free (ptr);
        return NULL;
    }

    if (NULL == ptr)
        return malloc (size);

    return realloc (ptr, size);
}

RState* r_state_new (RAllocFunc alloc_fn, rpointer aux)
{
    RState* r;
    RState zero = { { 0 } };

    r = alloc_fn (NULL, NULL, sizeof (RState));

    if (!r)
        goto exit;

    *r = zero;

    /* Initialize memory allocator */
    r->alloc_fn = alloc_fn;
    r->alloc_aux = aux;

    /* Initialize error handling facilities */
    r->last_error = R_UNDEFINED;

    gc_init (r);

    init_builtin_types (r);
    init_global_objects (r);

    vm_init (r);
    gc_enable (r);

exit:
    return r;
}

RState* r_state_open ()
{
    return r_state_new (default_alloc_fn, NULL);
}

void r_state_free (RState* r)
{
    r->last_error          = R_UNDEFINED;
    r->current_input_port  = R_UNDEFINED;
    r->current_output_port = R_UNDEFINED;
    r->current_error_port  = R_UNDEFINED;

    vm_finish (r);
    gc_finish (r);

    r_free (r, r);
}
