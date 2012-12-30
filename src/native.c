#include "detail/state.h"
#include "rose/native.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/vm.h"

#include <assert.h>
#include <stdarg.h>

struct RNative {
    R_OBJECT_HEADER
    rsexp name;
    RNativeProc proc;
    rsize required;
    rsize optional;
    rbool rest_p;
};

#define native_to_sexp(obj)   (r_cast (rsexp, (obj)))
#define native_from_sexp(obj) (r_cast (RNative*, (obj)))

static rbool validate_arity (RState* state, RNative* native, rsexp args)
{
    rsize length;

    assert (!(native->optional > 0 && native->rest_p));

    length = r_uint_from_sexp (r_length (state, args));

    return native->rest_p
           ? length >= native->required
           : length >= native->required &&
             length <= native->required + native->optional;
}

rsexp r_native_new (RState* state,
                    rconstcstring name,
                    RNativeProc proc,
                    rsize required,
                    rsize optional,
                    rbool rest_p)
{
    RNative* native = r_object_new (state, RNative, R_TAG_NATIVE_PROC);

    native->name     = r_symbol_new (state, name);
    native->proc     = proc;
    native->required = required;
    native->optional = optional;
    native->rest_p   = rest_p;

    return native_to_sexp (native);
}

rbool r_native_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_NATIVE_PROC;
}

static rsexp native_write (RState* state, rsexp port, rsexp obj)
{
    return r_port_write (state, port, native_from_sexp (obj)->name);
}

static rsexp native_display (RState* state, rsexp port, rsexp obj)
{
    return r_port_format (state, port,
            "#<native-procedure:~s>",native_from_sexp (obj)->name);
}

void init_native_type_info (RState* state)
{
    RTypeInfo type = { 0 };

    type.size         = sizeof (RNative);
    type.name         = "native-procedure";
    type.ops.write    = native_write;
    type.ops.display  = native_display;
    type.ops.eqv_p    = NULL;
    type.ops.equal_p  = NULL;
    type.ops.mark     = NULL;
    type.ops.finalize = NULL;

    init_builtin_type (state, R_TAG_NATIVE_PROC, &type);
}

rsexp r_native_apply (RState* state, rsexp obj, rsexp args)
{
    RNative* native;

    native = native_from_sexp (obj);

    if (!validate_arity (state, native, args)) {
        r_error_code (state, R_ERR_WRONG_ARG_NUM, obj);
        return R_FAILURE;
    }

    return native->proc (state, args);
}

void r_vmatch_args (RState* state,
                    rsexp args,
                    rsize required,
                    rsize optional,
                    rbool rest_p,
                    va_list va)
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

void r_match_args (RState* state,
                   rsexp args,
                   rsize required,
                   rsize optional,
                   rbool rest_p, ...)
{
    va_list va;

    assert (!(optional > 0 && rest_p));

    va_start (va, rest_p);
    r_vmatch_args (state, args, required, optional, rest_p, va);
    va_end (va);
}
