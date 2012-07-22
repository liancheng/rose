#include "detail/state.h"
#include "detail/sexp.h"
#include "rose/number.h"
#include "rose/port.h"

#include <gc/gc.h>

void r_register_pair_type   (RState* state);
void r_register_symbol_type (RState* state);

static void r_bool_write (rsexp port, rsexp obj)
{
    r_port_puts (port, (r_false_p (obj) ? "#f" : "#t"));
}

static void r_register_bool_type (RState* state)
{
    static RType type = {
        .size    = 0,
        .name    = "boolean",
        .write   = r_bool_write,
        .display = r_bool_write,
    };

    state->types [R_BOOL_TAG] = &type;
}

static void r_special_const_write (rsexp port, rsexp obj)
{
    static char* str[] = {
        "()",
        "#<eof>",
        "#<undefined>",
        "#<unspecified>",
    };

    r_port_puts (port, str [obj >> R_TAG_BITS]);
}

static void r_special_const_display (rsexp port, rsexp obj)
{
    r_port_puts (port, r_null_p (obj) ? "()" : "");
}

static void r_register_special_const_type (RState* state)
{
    static RType type = {
        .size    = 0,
        .name    = "special-const",
        .write   = r_special_const_write,
        .display = r_special_const_display,
    };

    state->types [R_SPECIAL_CONST_TAG] = &type;
}

static void r_smi_write (rsexp port, rsexp obj)
{
    r_port_printf (port, "%d", r_int_from_sexp (obj));
}

static void r_register_smi_type (RState* state)
{
    static RType type = {
        .size    = 0,
        .name    = "small-integer",
        .write   = r_smi_write,
        .display = r_smi_write,
    };

    state->types [R_SMI_EVEN_TAG] = &type;
    state->types [R_SMI_ODD_TAG]  = &type;
}

static void r_char_write (rsexp port, rsexp obj)
{
    char ch = r_char_from_sexp (obj);
    char* name = NULL;

    switch (ch) {
        case ' ':    name = "space";     break;
        case '\t':   name = "tab";       break;
        case '\n':   name = "newline";   break;
        case '\r':   name = "return";    break;
        case '\0':   name = "null";      break;
        case '\a':   name = "alarm";     break;
        case '\b':   name = "backspace"; break;
        case '\x1b': name = "escape";    break;
        case '\x7f': name = "delete";    break;
    }

    r_port_puts (port, "#\\");

    if (name)
        r_port_puts (port, name);
    else
        r_write_char (port, ch);
}

static void r_char_display (rsexp port, rsexp obj)
{
    r_write_char (port, r_char_from_sexp (obj));
}

static void r_register_char_type (RState* state)
{
    static RType type = {
        .size    = 0,
        .name    = "character",
        .write   = r_char_write,
        .display = r_char_display,
    };

    state->types [R_CHAR_TAG] = &type;
}

RType* r_sexp_get_type (RState* state, rsexp obj)
{
    return (r_boxed_p (obj))
           ? R_SEXP_TYPE (obj)
           : state->types [R_GET_TAG (obj)];
}

void r_register_types (RState* state)
{
    r_register_bool_type (state);
    r_register_special_const_type (state);
    r_register_pair_type (state);
    r_register_symbol_type (state);
    r_register_smi_type (state);
    r_register_char_type (state);
}
