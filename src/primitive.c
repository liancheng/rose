#include "detail/primitive.h"
#include "detail/state.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/vm.h"

#include <assert.h>
#include <stdarg.h>

struct RPrimitive {
    R_OBJECT_HEADER

    rsexp name;
    RPrimitiveFunc func;
    rsize required;
    rsize optional;
    rbool rest_p;
};

#define primitive_to_sexp(obj)   (r_cast (rsexp, (obj)))
#define primitive_from_sexp(obj) (r_cast (RPrimitive*, (obj)))

static rbool validate_arity (RState* r, RPrimitive* prim, rsexp args)
{
    rsize length;

    /* Optional arguments and rest argument cannot co-exist */
    assert (!(prim->optional > 0 && prim->rest_p));

    length = r_uint_from_sexp (r_length (r, args));

    if (length < prim->required)
        return FALSE;

    return prim->rest_p
           ? length >= prim->required
           : length >= prim->required &&
             length <= prim->required + prim->optional;
}

static void r_vmatch_args (RState* r, rsexp args, rsize required,
                           rsize optional, rbool rest_p, va_list va)
{
    rsize i;

    for (i = 0; i < required; ++i) {
        *(va_arg (va, rsexp*)) = r_car (args);
        args = r_cdr (args);
    }

    if (rest_p)
        *(va_arg (va, rsexp*)) = args;
    else
        for (i = 0; i < optional; ++i)
            if (r_null_p (args))
                *(va_arg (va, rsexp*)) = R_UNDEFINED;
            else {
                *(va_arg (va, rsexp*)) = r_car (args);
                args = r_cdr (args);
            }
}

rsexp r_primitive_new (RState* r, rconstcstring name, RPrimitiveFunc func,
                       rsize required, rsize optional, rbool rest_p)
{
    RPrimitive* prim = r_object_new (r, RPrimitive, R_TAG_PRIMITIVE);

    prim->name     = r_intern (r, name);
    prim->func     = func;
    prim->required = required;
    prim->optional = optional;
    prim->rest_p   = rest_p;

    return primitive_to_sexp (prim);
}

rbool r_primitive_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_PRIMITIVE;
}

static rsexp primitive_write (RState* r, rsexp port, rsexp obj)
{
    return r_port_format (r, port, "#<primitive:~s>",
                          primitive_from_sexp (obj)->name);
}

static void primitive_mark (RState* r, rsexp obj)
{
    r_gc_mark (r, primitive_from_sexp (obj)->name);
}

rsexp r_primitive_apply (RState* r, rsexp obj, rsexp args)
{
    RPrimitive* prim;

    prim = primitive_from_sexp (obj);

    if (!validate_arity (r, prim, args)) {
        r_error_code (r, R_ERR_WRONG_ARG_NUM, obj);
        return R_FAILURE;
    }

    return prim->func (r, args);
}

void r_match_args (RState* r,
                   rsexp args,
                   rsize required,
                   rsize optional,
                   rbool rest_p, ...)
{
    va_list va;

    assert (!(optional > 0 && rest_p));

    va_start (va, rest_p);
    r_vmatch_args (r, args, required, optional, rest_p, va);
    va_end (va);
}

RTypeInfo primitive_type = {
    .size = sizeof (RPrimitive),
    .name = "primitive",
    .ops = {
        .write = primitive_write,
        .display = primitive_write,
        .mark = primitive_mark
    }
};
