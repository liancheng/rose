#include "detail/gc.h"
#include "detail/sexp.h"
#include "detail/state.h"
#include "detail/vm.h"
#include "rose/error.h"
#include "rose/io.h"
#include "rose/gc.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define GC_ARENA_CHUNK_SIZE     1024
#define GC_DEFAULT_THRESHOLD    1024

#define gc_white_p(obj) ((obj)->gc_color == GC_COLOR_WHITE)
#define gc_gray_p(obj)  ((obj)->gc_color == GC_COLOR_GRAY)
#define gc_black_p(obj) ((obj)->gc_color == GC_COLOR_BLACK)

#define gc_paint_white(obj) ((obj)->gc_color = GC_COLOR_WHITE)
#define gc_paint_gray(obj)  ((obj)->gc_color = GC_COLOR_GRAY)
#define gc_paint_black(obj) ((obj)->gc_color = GC_COLOR_BLACK)

#ifndef NDEBUG

#endif

static inline void gc_scope_protect (RState* r, RObject* obj)
{
    RGc* gc = &r->gc;

    if (gc->arena_index == gc->arena_size) {
        gc->arena_size += GC_ARENA_CHUNK_SIZE;
        gc->arena = r_realloc (r,
                               gc->arena,
                               gc->arena_size * sizeof (rsexp));
    }

    gc->arena [gc->arena_index++] = object_to_sexp (obj);
}

static inline void gc_prepend_to_gray_list (RState* r, RObject* obj)
{
    RGc* gc = &r->gc;

    obj->gray_next = gc->gray_list;
    gc->gray_list = obj;
}

static inline void gc_prepend_to_chrono_list (RState* r, RObject* obj)
{
    RGc* gc = &r->gc;

    obj->chrono_next = gc->chrono_list;
    gc->chrono_list = obj;
}

static inline void gc_mark (RState* r, RObject* obj)
{
    if (!gc_white_p (obj) || gc_gray_p (obj))
        return;

    gc_paint_gray (obj);
    gc_prepend_to_gray_list (r, obj);
}

static inline void gc_mark_arena (RState* r)
{
    rsize i;
    RGc* gc = &r->gc;

    for (i = 0u; i < gc->arena_index; ++i)
        r_gc_mark (r, gc->arena [i]);
}

static inline void gc_mark_vm (RState* r)
{
    RVm* vm = &r->vm;

    r_gc_mark (r, vm->args);
    r_gc_mark (r, vm->env);
    r_gc_mark (r, vm->next);
    r_gc_mark (r, vm->stack);
    r_gc_mark (r, vm->value);
}

static inline void gc_scan_phase (RState* r)
{
    gc_mark_arena (r);
    gc_mark_vm (r);
}

static inline void gc_mark_children (RState* r, RObject* obj)
{
    RTypeInfo* type;
    RGcMark mark;

    type = r_type_info (r, object_to_sexp (obj));
    mark = type->ops.mark;

    if (mark)
        mark (r, object_to_sexp (obj));

    gc_paint_black (obj);
}

static inline void gc_mark_phase (RState* r)
{
    RObject*  obj;
    RGc* gc = &r->gc;

    while (gc->gray_list) {
        obj = gc->gray_list;
        gc->gray_list = obj->gray_next;
        gc_mark_children (r, obj);
    }
}

static inline void gc_sweep_phase (RState* r)
{
    RGc* gc;
    RObject** head;
    RObject* obj;

    gc = &r->gc;
    gc->n_live_after_mark = 0u;

    for (head = &gc->chrono_list; *head; ) {
        obj = *head;

        if (gc_white_p (obj)) {
            *head = obj->chrono_next;
            r_object_free (r, obj);
        }
        else {
            gc_paint_white (obj);
            gc->n_live_after_mark++;
            head = &obj->chrono_next;
        }
    }

    gc->n_live = gc->n_live_after_mark;
}

void gc_init (RState* r)
{
    RGc* gc = &r->gc;

    gc->enabled = FALSE;

    gc->arena_size = GC_ARENA_CHUNK_SIZE;
    gc->arena_index = 0u;
    gc->arena_last_index = 0u;
    gc->arena = r_new_array (r, rsexp, gc->arena_size);

    gc->n_live = 0u;
    gc->n_live_after_mark = 0u;
    gc->threshold = GC_DEFAULT_THRESHOLD;

    gc->gray_list = NULL;
    gc->chrono_list = NULL;

    assert (gc->arena);
}

void gc_finish (RState* r)
{
    gc_scope_reset (r);

    r_full_gc (r);
    r_free (r, r->gc.arena);
}

void gc_enable (RState* r)
{
    r->gc.enabled = TRUE;
}

void gc_disable (RState* r)
{
    r->gc.enabled = FALSE;
}

rbool gc_enabled_p (RState* r)
{
    return r->gc.enabled;
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
    if (gc_enabled_p (r)) {
        gc_scan_phase (r);
        gc_mark_phase (r);
        gc_sweep_phase (r);
    }
}

rpointer r_realloc (RState* r, rpointer ptr, rsize size)
{
    rpointer res = r->alloc_fn (r->alloc_aux, ptr, size);

    if (!res) {
        r_error_no_memory (r);
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
    RGc* gc;
    RTypeInfo* type;
    RObject* obj;

    assert (type_tag > R_TAG_BOXED && type_tag < R_TAG_MAX &&
            "invalid boxed type tag");

    gc = &r->gc;

    /* Allocated a lot objects, trigger GC */
    if (gc->n_live - gc->n_live_after_mark > gc->threshold)
        r_full_gc (r);

    type = &r->builtin_types [type_tag];
    obj = r_alloc (r, type->size);

    /* If out of memory... */
    if (obj == NULL) {
        /* then perform GC and retry */
        r_full_gc (r);
        obj = r_alloc (r, type->size);

        if (obj == NULL) {
            r_error_no_memory (r);
            return NULL;
        }
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
    RTypeInfo* type;
    RGcFinalize finalize;

    type = r_type_info (r, object_to_sexp (obj));
    finalize = type->ops.finalize;

    if (finalize)
        finalize (r, obj);

    r_free (r, obj);
}
