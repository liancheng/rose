#include "detail/state.h"
#include "detail/sexp.h"
#include "rose/error.h"
#include "rose/memory.h"
#include "rose/number.h"
#include "rose/port.h"

#include <gc/gc.h>

static void write_bool (RState* state, rsexp port, rsexp obj)
{
    r_port_puts (port, (r_false_p (obj) ? "#f" : "#t"));
}

static void write_special_const (RState* state, rsexp port, rsexp obj)
{
    static rconstcstring str[] = {
        "()",
        "#<eof>",
        "#<undefined>",
        "#<unspecified>",
    };

    r_port_puts (port, str [obj >> R_TAG_BITS]);
}

static void display_special_const (RState* state, rsexp port, rsexp obj)
{
    r_port_puts (port, r_null_p (obj) ? "()" : "");
}

static void write_smi (RState* state, rsexp port, rsexp obj)
{
    r_port_printf (state, port, "%d", r_int_from_sexp (obj));
}

static void write_char (RState* state, rsexp port, rsexp obj)
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

    r_port_puts (port, "#\\");

    if (name)
        r_port_puts (port, name);
    else
        r_write_char (port, ch);
}

static void display_char (RState* state, rsexp port, rsexp obj)
{
    r_write_char (port, r_char_from_sexp (obj));
}

RTypeInfo* init_bool_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = 0;
    type->name         = "boolean";
    type->ops.write    = write_bool;
    type->ops.display  = write_bool;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = NULL;
    type->ops.destruct = NULL;

    return type;
}

RTypeInfo* init_char_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = 0;
    type->name         = "character";
    type->ops.write    = write_char;
    type->ops.display  = display_char;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = NULL;
    type->ops.destruct = NULL;

    return type;
}

RTypeInfo* init_smi_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = 0;
    type->name         = "small-integer";
    type->ops.write    = write_smi;
    type->ops.display  = write_smi;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = NULL;
    type->ops.destruct = NULL;

    return type;
}

RTypeInfo* init_special_const_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = 0;
    type->name         = "special-const";
    type->ops.write    = write_special_const;
    type->ops.display  = display_special_const;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = NULL;
    type->ops.destruct = NULL;

    return type;
}

ruint r_type_tag (rsexp obj)
{
    return r_boxed_p (obj)
           ? r_cast (RObject*, obj)->type_tag
           : R_GET_TAG (obj);
}

RTypeInfo* r_type_info (RState* state, rsexp obj)
{
    return (r_boxed_p (obj))
           ? R_SEXP_TYPE (obj)
           : state->types [R_GET_TAG (obj)];
}

// TODO remove me when the GC mechanism is ready
static void finalize_object (rpointer obj, rpointer client_data)
{
    RObjDestruct destruct = r_cast (RObject*, obj)->type_info->ops.destruct;
    RState*      state    = r_cast (RState*, client_data);

    destruct (state, obj);
}

RObject* r_object_alloc (RState* state, RTypeTag type_tag)
{
    RTypeInfo* type_info = state->types [type_tag];
    RObject* obj = r_alloc (state, type_info->size);

    if (obj == NULL) {
        // TODO trigger GC
        r_error (state, "Out of memory");
    }

    obj->type_info = type_info;
    obj->type_tag = type_tag;

    // TODO remove me when the GC mechanism is ready
    GC_REGISTER_FINALIZER ((rpointer) obj, finalize_object, state, NULL, NULL);

    return obj;
}
