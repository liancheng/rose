#include "detail/state.h"
#include "detail/sexp.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/number.h"
#include "rose/port.h"

#include <gc/gc.h>

static rsexp write_special_const (RState* state, rsexp port, rsexp obj)
{
    static rconstcstring str[] = {
        "#f", "#t", "()", "#<eof>", "#<unspecified>", "#<undefined>",
    };

    return r_port_puts (state, port, str [obj >> R_TAG_BITS]);
}

static rsexp display_special_const (RState* state, rsexp port, rsexp obj)
{

    static rconstcstring str[] = {
        "#f", "#t", "()", "", "", "",
    };

    return r_port_puts (state, port, str [obj >> R_TAG_BITS]);
}

static rsexp write_smi (RState* state, rsexp port, rsexp obj)
{
    return r_port_printf (state, port, "%d", r_int_from_sexp (obj));
}

static rsexp write_char (RState* state, rsexp port, rsexp obj)
{
    char ch = r_char_from_sexp (obj);
    rcstring name = NULL;

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

    ensure (r_port_puts (state, port, "#\\"));

    if (name)
        ensure (r_port_puts (state, port, name));
    else
        ensure (r_port_write_char (state, port, ch));

    return R_UNSPECIFIED;
}

static rsexp display_char (RState* state, rsexp port, rsexp obj)
{
    return r_port_write_char (state, port, r_char_from_sexp (obj));
}

void init_char_type_info (RState* state)
{
    RTypeInfo type = { 0 };

    type.size         = 0;
    type.name         = "character";
    type.ops.write    = write_char;
    type.ops.display  = display_char;

    init_builtin_type (state, R_TAG_CHAR, &type);
}

void init_smi_type_info (RState* state)
{
    RTypeInfo type = { 0 };

    type.size         = 0;
    type.name         = "small-integer";
    type.ops.write    = write_smi;
    type.ops.display  = write_smi;

    init_builtin_type (state, R_TAG_SMI_EVEN, &type);
    init_builtin_type (state, R_TAG_SMI_ODD,  &type);
}

void init_special_const_type_info (RState* state)
{
    RTypeInfo type = { 0 };

    type.size         = 0;
    type.name         = "special-constant";
    type.ops.write    = write_special_const;
    type.ops.display  = display_special_const;

    init_builtin_type (state, R_TAG_SPECIAL_CONST, &type);
}

ruint r_type_tag (rsexp obj)
{
    return r_boxed_p (obj)
           ? r_cast (RObject*, obj)->type_tag
           : r_get_tag (obj);
}

RTypeInfo* r_type_info (RState* state, rsexp obj)
{
    return &state->builtin_types [r_type_tag (obj)];
}
