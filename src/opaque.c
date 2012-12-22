#include "detail/state.h"
#include "rose/gc.h"
#include "rose/opaque.h"
#include "rose/port.h"
#include "rose/sexp.h"

#include <assert.h>

typedef struct ROpaque ROpaque;

struct ROpaque {
    R_OBJECT_HEADER
    rpointer          opaque;
    ROpaqueGcMark     mark;
    ROpaqueGcFinalize finalize;
};

#define opaque_from_sexp(obj)   (r_cast (ROpaque*, (obj)))
#define opaque_to_sexp(obj)     (r_cast (rsexp, (obj)))

static rsexp opaque_write (RState* state, rsexp port, rsexp obj)
{
    ROpaque* opaque = opaque_from_sexp (obj);
    return r_port_printf (state, port, "#<opaque:%p>", opaque->opaque);
}

static rbool opaque_equal_p (RState* state, rsexp lhs, rsexp rhs)
{
    ROpaque* lhs_obj = opaque_from_sexp (lhs);
    ROpaque* rhs_obj = opaque_from_sexp (rhs);

    return lhs_obj->opaque == rhs_obj->opaque;
}

static void opaque_mark (RState* state, rsexp obj)
{
    ROpaque* opaque = opaque_from_sexp (obj);

    if (opaque->mark)
        opaque->mark (state, opaque->opaque);
}

static void opaque_finalize (RState* state, RObject* obj)
{
    ROpaque* opaque = r_cast (ROpaque*, obj);

    if (opaque->finalize)
        opaque->finalize (state, opaque->opaque);
}

void init_opaque_type_info (RState* state)
{
    RTypeInfo type = { 0 };

    type.size         = sizeof (ROpaque);
    type.name         = "opaque";
    type.ops.write    = opaque_write;
    type.ops.display  = opaque_write;
    type.ops.eqv_p    = NULL;
    type.ops.equal_p  = opaque_equal_p;
    type.ops.mark     = opaque_mark;
    type.ops.finalize = opaque_finalize;

    init_builtin_type (state, R_TAG_OPAQUE, &type);
}

rsexp r_opaque_new (RState*           state,
                    rpointer          opaque,
                    ROpaqueGcMark     mark_fn,
                    ROpaqueGcFinalize finalize_fn)
{
    ROpaque* res = r_object_new (state, ROpaque, R_TAG_OPAQUE);

    if (!res)
        return R_FAILURE;

    res->opaque   = opaque;
    res->mark     = mark_fn;
    res->finalize = finalize_fn;

    return object_to_sexp (res);
}

rbool r_opaque_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_OPAQUE;
}

rpointer r_opaque_get (rsexp obj)
{
    assert (r_opaque_p (obj));
    return opaque_from_sexp (obj)->opaque;
}
