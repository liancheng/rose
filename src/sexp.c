#include "detail/state.h"
#include "detail/sexp.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/number.h"
#include "rose/port.h"

#include <gc/gc.h>

static rsexp write_special_const (RState* r, rsexp port, rsexp obj)
{
    static rconstcstring str[] = {
        "#f", "#t", "()", "#<eof>", "#<unspecified>",
        "#<undefined>", "#<failure>"
    };

    return r_port_puts (r, port, str [obj >> R_TAG_BITS]);
}

static rsexp display_special_const (RState* r, rsexp port, rsexp obj)
{

    static rconstcstring str[] = {
        "#f", "#t", "()", "", "", "", ""
    };

    return r_port_puts (r, port, str [obj >> R_TAG_BITS]);
}

static rsexp write_smi (RState* r, rsexp port, rsexp obj)
{
    return r_port_printf (r, port, "%d", r_int_from_sexp (obj));
}

static rsexp write_char (RState* r, rsexp port, rsexp obj)
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

    ensure (r_port_puts (r, port, "#\\"));

    if (name)
        ensure (r_port_puts (r, port, name));
    else
        ensure (r_port_write_char (r, port, ch));

    return R_UNSPECIFIED;
}

static rsexp display_char (RState* r, rsexp port, rsexp obj)
{
    return r_port_write_char (r, port, r_char_from_sexp (obj));
}

ruint r_type_tag (rsexp obj)
{
    return r_boxed_p (obj)
           ? r_cast (RObject*, obj)->type_tag
           : r_get_tag (obj);
}

RTypeInfo* r_type_info (RState* r, rsexp obj)
{
    return &r->builtin_types [r_type_tag (obj)];
}

RTypeInfo char_type = {
    .size = 0,
    .name = "character",
    .ops = {
        .write = write_char,
        .display = display_char
    }
};

RTypeInfo small_int_type = {
    .size = 0,
    .name = "small-integer",
    .ops = {
        .write = write_smi,
        .display = write_smi
    }
};

RTypeInfo special_const_type = {
    .size = 0,
    .name = "special-constant",
    .ops = {
        .write = write_special_const,
        .display = display_special_const
    }
};
