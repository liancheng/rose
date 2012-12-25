#include "detail/gc.h"
#include "detail/sexp.h"
#include "detail/state.h"

typedef struct RClosure RClosure;

struct RClosure {
    R_OBJECT_HEADER
    rsexp body;
    rsexp env;
    rsexp vars;
};

#define closure_to_sexp(obj)    (r_cast (rsexp, (obj)))
#define closure_from_sexp(obj)  (r_cast (RClosure*, (obj)))

static void closure_mark (RState* state, rsexp obj)
{
    RClosure* closure = closure_from_sexp (obj);

    r_gc_mark (state, closure->body);
    r_gc_mark (state, closure->env);
    r_gc_mark (state, closure->vars);
}

void init_closure_type_info (RState* state)
{
    RTypeInfo type = { 0 };

    type.name         = "procedure";
    type.size         = sizeof (RClosure);
    type.ops.display  = NULL;
    type.ops.write    = NULL;
    type.ops.eqv_p    = NULL;
    type.ops.equal_p  = NULL;
    type.ops.finalize = NULL;
    type.ops.mark     = closure_mark;

    init_builtin_type (state, R_TAG_CLOSURE, &type);
}

rbool r_closure_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_CLOSURE;
}

rsexp r_closure_new (RState* state, rsexp body, rsexp env, rsexp vars)
{
    RClosure* closure = r_object_new (state, RClosure, R_TAG_CLOSURE);

    if (!closure)
        return R_FAILURE;

    closure->body = body;
    closure->env  = env;
    closure->vars = vars;

    return object_to_sexp (closure);
}

rsexp r_closure_body (rsexp obj)
{
    return closure_from_sexp (obj)->body;
}

rsexp r_closure_env (rsexp obj)
{
    return closure_from_sexp (obj)->env;
}

rsexp r_closure_vars (rsexp obj)
{
    return closure_from_sexp (obj)->vars;
}
