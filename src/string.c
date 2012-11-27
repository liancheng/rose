#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/memory.h"
#include "rose/port.h"
#include "rose/string.h"

#include <assert.h>
#include <string.h>

struct RString {
    R_OBJECT_HEADER
    rsize    length;
    rcstring data;
};

#define STRING_FROM_SEXP(obj)   (r_cast (RString*, (obj)))
#define STRING_TO_SEXP(string)  (r_cast (rsexp, (string)))

static void write_string (RState* state, rsexp port, rsexp obj)
{
    rcstring p;

    r_write_char (port, '"');

    for (p = STRING_FROM_SEXP (obj)->data; *p; ++p)
        if ('"' == *p)
            r_port_puts (port, "\\\"");
        else
            r_write_char (port, *p);

    r_write_char (port, '"');
}

static void display_string (RState* state, rsexp port, rsexp obj)
{
    r_port_puts (port, STRING_FROM_SEXP (obj)->data);
}

static void destruct_string (RState* state, RObject* obj)
{
    RString* str = r_cast (RString*, obj);
    r_free (state, str->data);
}

RTypeInfo* init_string_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (RString);
    type->name         = "string";
    type->ops.write    = write_string;
    type->ops.display  = display_string;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = r_string_equal_p;
    type->ops.mark     = NULL;
    type->ops.destruct = destruct_string;

    return type;
}

rsexp r_string_new (RState* state, rconstcstring str)
{
    RString* res = r_object_new (state, RString, R_STRING_TAG);

    res->length = strlen (str) + 1;
    res->data = r_strdup (state, str);

    return STRING_TO_SEXP (res);
}

rbool r_string_p (rsexp obj)
{
    return r_type_tag (obj) == R_STRING_TAG;
}

rconstcstring r_string_to_cstr (rsexp obj)
{
    return STRING_FROM_SEXP (obj)->data;
}

rbool r_string_equal_p (RState* state, rsexp lhs, rsexp rhs)
{
    RString* lhs_str;
    RString* rhs_str;

    if (!r_string_p (lhs) || !r_string_p (rhs))
        return FALSE;

    lhs_str = STRING_FROM_SEXP (lhs);
    rhs_str = STRING_FROM_SEXP (rhs);

    return lhs_str->length == rhs_str->length
        && 0 == memcmp (lhs_str->data,
                        rhs_str->data,
                        lhs_str->length * sizeof (char));
}

rint r_string_byte_count (rsexp obj)
{
    assert (r_string_p (obj));
    return STRING_FROM_SEXP (obj)->length;
}

rint r_string_length (rsexp obj)
{
    assert (r_string_p (obj));
    return STRING_FROM_SEXP (obj)->length;
}
