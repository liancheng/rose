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

static rsexp opaque_write (RState* r, rsexp port, rsexp obj)
{
    ROpaque* opaque = opaque_from_sexp (obj);
    return r_port_printf (r, port, "#<opaque:%p>", opaque->opaque);
}

static rbool opaque_equal_p (RState* r, rsexp lhs, rsexp rhs)
{
    ROpaque* lhs_obj = opaque_from_sexp (lhs);
    ROpaque* rhs_obj = opaque_from_sexp (rhs);

    return lhs_obj->opaque == rhs_obj->opaque;
}

static void opaque_mark (RState* r, rsexp obj)
{
    ROpaque* opaque = opaque_from_sexp (obj);

    if (opaque->mark)
        opaque->mark (r, opaque->opaque);
}

static void opaque_finalize (RState* r, RObject* obj)
{
    ROpaque* opaque = r_cast (ROpaque*, obj);

    if (opaque->finalize)
        opaque->finalize (r, opaque->opaque);
}

rsexp r_opaque_new (RState* r,
                    rpointer opaque,
                    ROpaqueGcMark mark_fn,
                    ROpaqueGcFinalize finalize_fn)
{
    ROpaque* res = r_object_new (r, ROpaque, R_TAG_OPAQUE);

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

RTypeInfo opaque_type = {
    .size = sizeof (ROpaque),
    .name = "opaque",
    .ops = {
        .write    = opaque_write,
        .display  = opaque_write,
        .equal_p  = opaque_equal_p,
        .mark     = opaque_mark,
        .finalize = opaque_finalize
    }
};
