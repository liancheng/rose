#include "detail/bytevector.h"
#include "detail/env.h"
#include "detail/error.h"
#include "detail/number.h"
#include "detail/pair.h"
#include "detail/port.h"
#include "detail/sexp.h"
#include "detail/state.h"
#include "detail/string.h"
#include "detail/symbol.h"
#include "detail/vector.h"

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

static void register_type_info (RState* state)
{
    state->types [R_BOOL_TAG         ] = init_bool_type_info          (state);
    state->types [R_BYTEVECTOR_TAG   ] = init_bytevector_type_info    (state);
    state->types [R_CHAR_TAG         ] = init_char_type_info          (state);
    state->types [R_ENV_TAG          ] = init_env_type_info           (state);
    state->types [R_ERROR_TAG        ] = init_error_type_info         (state);
    state->types [R_FIXNUM_TAG       ] = init_fixnum_type_info        (state);
    state->types [R_FLONUM_TAG       ] = init_flonum_type_info        (state);
    state->types [R_PAIR_TAG         ] = init_pair_type_info          (state);
    state->types [R_PORT_TAG         ] = init_port_type_info          (state);
    state->types [R_SMI_EVEN_TAG     ] = init_smi_type_info           (state);
    state->types [R_SMI_ODD_TAG      ] = init_smi_type_info           (state);
    state->types [R_SPECIAL_CONST_TAG] = init_special_const_type_info (state);
    state->types [R_STRING_TAG       ] = init_string_type_info        (state);
    state->types [R_SYMBOL_TAG       ] = init_symbol_type_info        (state);
    state->types [R_VECTOR_TAG       ] = init_vector_type_info        (state);
}

RState* r_state_new (RAllocFunc alloc_fn, rpointer aux)
{
    RState* state = calloc (sizeof (RState), 1u);

    /* Initialize memory allocator first */
    state->alloc_fn  = alloc_fn;
    state->alloc_aux = aux;

    register_type_info (state);

    state->env = r_env_new (state);
    state->current_input_port = r_stdin_port (state);
    state->current_output_port = r_stdout_port (state);
    state->error_jmp = NULL;

    /* Symbol table must be initialized before keywords registration */
    state->symbol_table = r_symbol_table_new (state);
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
