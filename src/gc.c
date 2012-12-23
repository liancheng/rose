#include "detail/gc.h"
#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/port.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define ARENA_CHUNK_SIZE 16

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

static void gc_scope_protect (RState* state, RObject* obj)
{
    RGcState* gc = &state->gc;

    if (gc->arena_index >= gc->arena_size) {
        gc->arena_size += ARENA_CHUNK_SIZE;
        gc->arena = r_realloc (state,
                               gc->arena,
                               gc->arena_size * sizeof (RObject*));
    }

    gc->arena [gc->arena_index++] = obj;
}

static void gc_prepend_to_gray_list (RState* state, RObject* obj)
{
    RGcState* gc = &state->gc;

    obj->gray_next = gc->gray_list;
    gc->gray_list  = obj;
}

static void gc_prepend_to_chrono_list (RState* state, RObject* obj)
{
    RGcState* gc = &state->gc;

    obj->chrono_next = gc->chrono_list;
    gc->chrono_list  = obj;
}

static void gc_mark (RState* state, RObject* obj)
{
    if (!gc_white_p (obj))
        return;

    gc_paint_gray (obj);
    gc_prepend_to_gray_list (state, obj);
}

static void gc_mark_arena (RState* state)
{
    rsize i;
    RGcState* gc = &state->gc;

    for (i = 0u; i < gc->arena_index; ++i)
        gc_mark (state, gc->arena [i]);
}

static void gc_scan_phase (RState* state)
{
    gc_mark_arena (state);
}

static void gc_mark_children (RState* state, RObject* obj)
{
    RTypeInfo* type;
    RGcMark    mark;

    type = r_type_info (state, object_to_sexp (obj));
    mark = type->ops.mark;

    if (mark)
        mark (state, object_to_sexp (obj));

    gc_paint_black (obj);
}

static void gc_mark_phase (RState* state)
{
    RObject*  obj;
    RGcState* gc = &state->gc;

    while (gc->gray_list) {
        obj = gc->gray_list;
        gc->gray_list = obj->gray_next;
        gc_mark_children (state, obj);
    }
}

static void gc_sweep_phase (RState* state)
{
    RObject   dummy = { 0 };
    RGcState* gc    = &state->gc;
    RObject*  obj   = gc->chrono_list;
    RObject*  prev  = &dummy;

    prev->chrono_next = obj;

    while (obj) {
        assert (!gc_gray_p (obj));

        if (gc_black_p (obj)) {
            gc_paint_white (obj);
            prev = obj;
            obj = obj->chrono_next;
        }
        else {
            prev->chrono_next = obj->chrono_next;
            r_object_free (state, obj);
            obj = prev->chrono_next;
        }
    }
}

void gc_state_init (RState* state)
{
    RGcState* gc = &state->gc;

    gc->arena_size       = ARENA_CHUNK_SIZE;
    gc->arena_index      = 0u;
    gc->arena_last_index = 0u;
    gc->arena            = r_new_array (state, RObject*, gc->arena_size);

    gc->gray_list        = NULL;
    gc->chrono_list      = NULL;

    assert (gc->arena);
}

void gc_state_destruct (RState* state)
{
    gc_scope_reset (state);

    r_full_gc (state);
    r_free (state, state->gc.arena);
}

void gc_scope_reset (RState* state)
{
    state->gc.arena_index = 0u;
    state->gc.arena_last_index = 0u;
}

void r_gc_scope_open (RState* state)
{
    state->gc.arena_last_index = state->gc.arena_index;
}

void r_gc_scope_close (RState* state)
{
    state->gc.arena_index = state->gc.arena_last_index;
}

void r_gc_scope_protect (RState* state, rsexp obj)
{
    if (r_boxed_p (obj))
        gc_scope_protect (state, object_from_sexp (obj));
}

void r_gc_mark (RState* state, rsexp obj)
{
    if (r_boxed_p (obj))
        gc_mark (state, object_from_sexp (obj));
}

void r_full_gc (RState* state)
{
    gc_scan_phase (state);
    gc_mark_phase (state);
    gc_sweep_phase (state);
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

rpointer r_realloc (RState* state, rpointer ptr, rsize size)
{
    rpointer res = state->alloc_fn (state->alloc_aux, ptr, size);

    if (!res) {
        r_set_last_error_x (state, R_ERROR_OOM);
        res = NULL;
    }

    return res;
}

rpointer r_alloc (RState* state, rsize size)
{
    return r_realloc (state, NULL, size);
}

rpointer r_calloc (RState* state, rsize element_size, rsize count)
{
    rpointer ptr = r_alloc (state, element_size * count);

    if (ptr)
        memset (ptr, 0, element_size * count);

    return ptr;
}

void r_free (RState* state, rpointer ptr)
{
    state->alloc_fn (state->alloc_aux, ptr, 0u);
}

RObject* r_object_alloc (RState* state, RTypeTag type_tag)
{
    RTypeInfo* type = &state->builtin_types [type_tag];
    RObject*   obj  = r_alloc (state, type->size);

    if (obj == NULL) {
        r_set_last_error_x (state, R_ERROR_OOM);
        return NULL;
    }

    obj->type_tag = type_tag;

    gc_paint_white (obj);
    gc_prepend_to_chrono_list (state, obj);
    gc_scope_protect (state, obj);

    return obj;
}

void r_object_free (RState* state, RObject* obj)
{
    RTypeInfo*  type;
    RGcFinalize finalize;

    type = r_type_info (state, object_to_sexp (obj));
    finalize = type->ops.finalize;

    if (finalize)
        finalize (state, obj);

    r_free (state, obj);
}
