#include "detail/gc.h"
#include "detail/sexp.h"
#include "detail/state.h"

typedef struct RProcedure RProcedure;

struct RProcedure {
    R_OBJECT_HEADER
    rsexp body;
    rsexp env;
    rsexp vars;
};

#define procedure_to_sexp(obj)   (r_cast (rsexp, (obj)))
#define procedure_from_sexp(obj) (r_cast (RProcedure*, (obj)))

static void procedure_mark (RState* state, rsexp obj)
{
    RProcedure* procedure = procedure_from_sexp (obj);

    r_gc_mark (state, procedure->body);
    r_gc_mark (state, procedure->env);
    r_gc_mark (state, procedure->vars);
}

void init_procedure_type_info (RState* state)
{
    RTypeInfo type = { 0 };

    type.name         = "procedure";
    type.size         = sizeof (RProcedure);
    type.ops.display  = NULL;
    type.ops.write    = NULL;
    type.ops.eqv_p    = NULL;
    type.ops.equal_p  = NULL;
    type.ops.finalize = NULL;
    type.ops.mark     = procedure_mark;

    init_builtin_type (state, R_TAG_PROCEDURE, &type);
}

rbool r_procedure_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_PROCEDURE;
}

rsexp r_procedure_new (RState* state, rsexp body, rsexp env, rsexp vars)
{
    RProcedure* procedure = r_object_new (state, RProcedure, R_TAG_PROCEDURE);

    if (!procedure)
        return R_FAILURE;

    procedure->body = body;
    procedure->env  = env;
    procedure->vars = vars;

    return object_to_sexp (procedure);
}

rsexp r_procedure_body (rsexp obj)
{
    return procedure_from_sexp (obj)->body;
}

rsexp r_procedure_env (rsexp obj)
{
    return procedure_from_sexp (obj)->env;
}

rsexp r_procedure_vars (rsexp obj)
{
    return procedure_from_sexp (obj)->vars;
}
