#include "detail/gc.h"
#include "detail/sexp.h"
#include "detail/state.h"
#include "detail/vm.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/port.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define GC_ARENA_CHUNK_SIZE     256
#define GC_DEFAULT_THRESHOLD    128

typedef enum {
    R_GC_COLOR_WHITE,
    R_GC_COLOR_GRAY,
    R_GC_COLOR_BLACK
}
RGcColor;

#define gc_white_p(obj)     ((obj)->gc_color == R_GC_COLOR_WHITE)
#define gc_gray_p(obj)      ((obj)->gc_color == R_GC_COLOR_GRAY)
#define gc_black_p(obj)     ((obj)->gc_color == R_GC_COLOR_BLACK)

#define gc_paint_white(obj) ((obj)->gc_color = R_GC_COLOR_WHITE)
#define gc_paint_gray(obj)  ((obj)->gc_color = R_GC_COLOR_GRAY)
#define gc_paint_black(obj) ((obj)->gc_color = R_GC_COLOR_BLACK)

static void gc_scope_protect (RState* r, RObject* obj)
{
    RGcState* gc = &r->gc;

    if (gc->arena_index >= gc->arena_size) {
#ifdef ROSE_DYNAMIC_ARENA
        gc->arena_size += GC_ARENA_CHUNK_SIZE;
        gc->arena = r_realloc (r,
                               gc->arena,
                               gc->arena_size * sizeof (RObject*));
#else
        fprintf (stderr, "arena index overflow\n");
        abort ();
#endif
    }

    gc->arena [gc->arena_index++] = obj;
}

static void gc_prepend_to_gray_list (RState* r, RObject* obj)
{
    RGcState* gc = &r->gc;

    obj->gray_next = gc->gray_list;
    gc->gray_list  = obj;
}

static void gc_prepend_to_chrono_list (RState* r, RObject* obj)
{
    RGcState* gc = &r->gc;

    obj->chrono_next = gc->chrono_list;
    gc->chrono_list  = obj;
}

static void gc_mark (RState* r, RObject* obj)
{
    if (!gc_white_p (obj) || gc_gray_p (obj))
        return;

    gc_paint_gray (obj);
    gc_prepend_to_gray_list (r, obj);
}

static void gc_mark_arena (RState* r)
{
    rsize i;
    RGcState* gc = &r->gc;

    for (i = 0u; i < gc->arena_index; ++i)
        gc_mark (r, gc->arena [i]);
}

static void gc_mark_vm (RState* r)
{
    RVm* vm = &r->vm;

    r_gc_mark (r, vm->args);
    r_gc_mark (r, vm->env);
    r_gc_mark (r, vm->next);
    r_gc_mark (r, vm->stack);
    r_gc_mark (r, vm->value);
}

static void gc_scan_phase (RState* r)
{
    gc_mark_arena (r);
    gc_mark_vm (r);
}

static void gc_mark_children (RState* r, RObject* obj)
{
    RTypeInfo* type;
    RGcMark    mark;

    type = r_type_info (r, object_to_sexp (obj));
    mark = type->ops.mark;

    if (mark)
        mark (r, object_to_sexp (obj));

    gc_paint_black (obj);
}

static void gc_mark_phase (RState* r)
{
    RObject*  obj;
    RGcState* gc = &r->gc;

    while (gc->gray_list) {
        obj = gc->gray_list;
        gc->gray_list = obj->gray_next;
        gc_mark_children (r, obj);
    }
}

static void gc_sweep_phase (RState* r)
{
    RObject   dummy = { 0 };
    RGcState* gc    = &r->gc;
    RObject*  obj   = gc->chrono_list;
    RObject*  prev  = &dummy;

    prev->chrono_next = obj;
    gc->n_live_after_mark = 0u;

    while (obj) {
        assert (!gc_gray_p (obj));

        if (gc_black_p (obj)) {
            gc_paint_white (obj);
            prev = obj;
            obj = obj->chrono_next;
            gc->n_live_after_mark++;
        }
        else {
            prev->chrono_next = obj->chrono_next;
            r_object_free (r, obj);
            obj = prev->chrono_next;
        }
    }

    gc->n_live = gc->n_live_after_mark;
}

void gc_init (RState* r)
{
    RGcState* gc = &r->gc;

    gc->arena_size        = GC_ARENA_CHUNK_SIZE;
    gc->arena_index       = 0u;
    gc->arena_last_index  = 0u;
    gc->arena             = r_new_array (r, RObject*, gc->arena_size);

    gc->n_live            = 0u;
    gc->n_live_after_mark = 0u;
    gc->threshold         = GC_DEFAULT_THRESHOLD;

    gc->gray_list         = NULL;
    gc->chrono_list       = NULL;

    assert (gc->arena);
}

void gc_finish (RState* r)
{
    gc_scope_reset (r);

    r_full_gc (r);
    r_free (r, r->gc.arena);
}

void gc_scope_reset (RState* r)
{
    r->gc.arena_index = 0u;
    r->gc.arena_last_index = 0u;
}

void r_gc_scope_open (RState* r)
{
    r->gc.arena_last_index = r->gc.arena_index;
}

void r_gc_scope_close (RState* r)
{
    r->gc.arena_index = r->gc.arena_last_index;
}

void r_gc_scope_protect (RState* r, rsexp obj)
{
    if (r_boxed_p (obj))
        gc_scope_protect (r, object_from_sexp (obj));
}

void r_gc_mark (RState* r, rsexp obj)
{
    if (r_boxed_p (obj))
        gc_mark (r, object_from_sexp (obj));
}

void r_full_gc (RState* r)
{
    gc_scan_phase (r);
    gc_mark_phase (r);
    gc_sweep_phase (r);
}

rpointer default_alloc_fn (rpointer aux, rpointer ptr, rsize size)
{
    if (0 == size) {
        free (ptr);
        return NULL;
    }

    if (NULL == ptr)
        return malloc (size);

    return realloc (ptr, size);
}

rpointer r_realloc (RState* r, rpointer ptr, rsize size)
{
    rpointer res = r->alloc_fn (r->alloc_aux, ptr, size);

    if (!res) {
        r_set_last_error_x (r, R_ERROR_OOM);
        res = NULL;
    }

    return res;
}

rpointer r_alloc (RState* r, rsize size)
{
    return r_realloc (r, NULL, size);
}

rpointer r_calloc (RState* r, rsize element_size, rsize count)
{
    rpointer ptr = r_alloc (r, element_size * count);

    if (ptr)
        memset (ptr, 0, element_size * count);

    return ptr;
}

void r_free (RState* r, rpointer ptr)
{
    r->alloc_fn (r->alloc_aux, ptr, 0u);
}

RObject* r_object_alloc (RState* r, RTypeTag type_tag)
{
    RGcState*  gc;
    RTypeInfo* type;
    RObject*   obj;

    gc = &r->gc;

    if (gc->n_live - gc->n_live_after_mark > gc->threshold)
        r_full_gc (r);

    type = &r->builtin_types [type_tag];
    obj  = r_alloc (r, type->size);

    if (obj == NULL) {
        r_set_last_error_x (r, R_ERROR_OOM);
        return NULL;
    }

    memset (r_cast (rpointer, obj), 0, type->size);

    obj->type_tag = type_tag;

    gc_paint_white (obj);
    gc_prepend_to_chrono_list (r, obj);
    gc_scope_protect (r, obj);

    gc->n_live++;

    return obj;
}

void r_object_free (RState* r, RObject* obj)
{
    RTypeInfo*  type;
    RGcFinalize finalize;

    type = r_type_info (r, object_to_sexp (obj));
    finalize = type->ops.finalize;

    if (finalize)
        finalize (r, obj);

    r_free (r, obj);
}
