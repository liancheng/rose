#include "detail/gc.h"
#include "detail/state.h"
#include "rose/io.h"
#include "rose/number.h"
#include "rose/primitive.h"

static rsexp np_gc_enable (RState* r, rsexp args)
{
    gc_enable (r);
    return R_UNSPECIFIED;
}

static rsexp np_gc_disable (RState* r, rsexp args)
{
    gc_disable (r);
    return R_UNSPECIFIED;
}

static rsexp np_gc_enabled_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (gc_enabled_p (r));
}

static rsexp np_full_gc (RState* r, rsexp args)
{
    r_full_gc (r);
    return R_UNSPECIFIED;
}

static rsexp np_gc_live_object_count (RState* r, rsexp args)
{
    return r_int_to_sexp (r->gc.n_live);
}

static rsexp np_gc_arena_size (RState* r, rsexp args)
{
    return r_int_to_sexp (r->gc.arena_size);
}

static rsexp np_gc_arena_index (RState* r, rsexp args)
{
    return r_int_to_sexp (r->gc.arena_index);
}

static rsexp np_gc_dump_live_objects (RState* r, rsexp args)
{
    RGc* gc;
    RObject* obj;

    for (gc = &r->gc, obj = gc->chrono_list; obj; obj = obj->chrono_next)
        r_format (r, "~s~%", object_to_sexp (obj));

    return R_UNSPECIFIED;
}

static rsexp np_gc_dump_arena (RState* r, rsexp args)
{
    RGc* gc;
    rsize i;

    for (gc = &r->gc, i = 0; i < gc->arena_index; ++i)
        r_format (r, "~s~%", gc->arena [i]);

    return R_UNSPECIFIED;
}

RPrimitiveDesc gc_primitives [] = {
    { "gc-enable",            np_gc_enable,            0, 0, FALSE },
    { "gc-disable",           np_gc_disable,           0, 0, FALSE },
    { "gc-enabled?",          np_gc_enabled_p,         0, 0, FALSE },
    { "full-gc",              np_full_gc,              0, 0, FALSE },
    { "gc-live-object-count", np_gc_live_object_count, 0, 0, FALSE },
    { "gc-arena-size",        np_gc_arena_size,        0, 0, FALSE },
    { "gc-arena-index",       np_gc_arena_index,       0, 0, FALSE },
    { "gc-dump-live-objects", np_gc_dump_live_objects, 0, 0, FALSE },
    { "gc-dump-arena",        np_gc_dump_arena,        0, 0, FALSE },
    { NULL }
};
