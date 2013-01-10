#include "detail/gc.h"
#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/procedure.h"

typedef struct RProcedure RProcedure;

struct RProcedure {
    R_OBJECT_HEADER
    rsexp body;
    rsexp env;
    rsexp formals;
};

#define procedure_to_sexp(obj)   (r_cast (rsexp, (obj)))
#define procedure_from_sexp(obj) (r_cast (RProcedure*, (obj)))

static void procedure_mark (RState* r, rsexp obj)
{
    RProcedure* procedure = procedure_from_sexp (obj);

    r_gc_mark (r, procedure->body);
    r_gc_mark (r, procedure->env);
    r_gc_mark (r, procedure->formals);
}

rbool r_procedure_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_PROCEDURE;
}

rsexp r_procedure_new (RState* r, rsexp body, rsexp env, rsexp formals)
{
    RProcedure* procedure = r_object_new (r, RProcedure, R_TAG_PROCEDURE);

    if (!procedure)
        return R_FAILURE;

    procedure->body = body;
    procedure->env = env;
    procedure->formals = formals;

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

rsexp r_procedure_formals (rsexp obj)
{
    return procedure_from_sexp (obj)->formals;
}

RTypeInfo procedure_type = {
    .name = "procedure",
    .size = sizeof (RProcedure),
    .ops = {
        .mark = procedure_mark
    }
};
