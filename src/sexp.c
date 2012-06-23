#include "detail/context.h"
#include "detail/sexp.h"
#include "rose/port.h"

#include <gc/gc.h>

void r_register_pair_type   (RContext* context);
void r_register_symbol_type (RContext* context);
void r_register_fixnum_type (RContext* context);
void r_register_flonum_type (RContext* context);

static void r_boolean_write (rsexp port, rsexp obj)
{
    r_port_puts (port, (r_false_p (obj) ? "#f" : "#t"));
}

static void r_register_boolean_type (RContext* context)
{
    RType* type = GC_NEW (RType);

    type->cell_size  = 0;
    type->name       = "boolean";
    type->write_fn   = r_boolean_write;
    type->display_fn = r_boolean_write;

    context->tc3_types [R_BOOLEAN_TAG] = type;
}

static void r_special_const_write (rsexp port, rsexp obj)
{
    static char* str[] = {
        "()",
        "#<eof>",
        "#<undefined>",
        "#<unspecified>",
    };

    r_port_puts (port, str [obj >> R_TC5_BITS]);
}

static void r_special_const_display (rsexp port, rsexp obj)
{
    r_port_puts (port, r_null_p (obj) ? "()" : "");
}

static void r_register_special_const_type (RContext* context)
{
    RType* type = GC_NEW (RType);

    type->cell_size  = 0;
    type->name       = "special-const";
    type->write_fn   = r_special_const_write;
    type->display_fn = r_special_const_display;

    context->tc5_types [R_SPECIAL_CONST_TAG >> R_TC3_BITS] = type;
}

static void r_int30_write (rsexp port, rsexp obj)
{
    r_port_printf (port, "%d", r_int_from_sexp (obj));
}

static void r_register_int30_type (RContext* context)
{
    RType* type = GC_NEW (RType);

    type->cell_size  = 0;
    type->name       = "small-integer";
    type->write_fn   = r_int30_write;
    type->display_fn = r_int30_write;

    context->tc3_types [R_INT30_EVEN_TAG] = type;
    context->tc3_types [R_INT30_ODD_TAG] = type;
}

static void r_character_write (rsexp port, rsexp obj)
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

static void r_character_display (rsexp port, rsexp obj)
{
    r_write_char (port, r_char_from_sexp (obj));
}

static void r_register_character_type (RContext* context)
{
    RType* type = GC_NEW (RType);

    type->cell_size  = 0;
    type->name       = "character";
    type->write_fn   = r_character_write;
    type->display_fn = r_character_display;

    context->tc5_types [R_CHARACTER_TAG >> R_TC3_BITS] = type;
}

RType* r_sexp_get_type (rsexp obj, RContext* context)
{
    rint tc3 = (obj & R_TC3_MASK);

    if (R_CELL_TAG == tc3)
        return R_CELL_TYPE (obj);

    if (R_TC5_TAG == tc3) {
        rint tc5 = (obj & R_TC5_MASK);
        return context->tc5_types [tc5 >> R_TC3_BITS];
    }

    return context->tc3_types [tc3];
}

void r_register_tc3_types (RContext* context)
{
    r_register_boolean_type (context);
    r_register_special_const_type (context);
    r_register_pair_type (context);
    r_register_symbol_type (context);
    r_register_int30_type (context);
    r_register_character_type (context);
    r_register_fixnum_type (context);
    r_register_flonum_type (context);
}
