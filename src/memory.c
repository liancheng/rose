#include "detail/state.h"
#include "rose/error.h"
#include "rose/memory.h"

#include <gc/gc.h>
#include <string.h>

// TODO remove me when the GC mechanism is ready
static void finalize_object (rpointer obj, rpointer client_data)
{
    RObjDestruct destruct = r_cast (RObject*, obj)->type_info->ops.destruct;
    RState*      state    = r_cast (RState*, client_data);

    destruct (state, obj);
}

rpointer default_alloc_fn (rpointer aux, rpointer ptr, rsize size)
{
    // Free the memory if size is 0.
    if (0 == size)
        return NULL;

    if (NULL == ptr) {
        return GC_MALLOC (size);
    }

    return GC_REALLOC (ptr, size);
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
    RTypeInfo* type = state->builtin_types [type_tag];
    RObject*   obj  = r_alloc (state, type->size);

    if (obj == NULL) {
        r_set_last_error_x (state, R_ERROR_OOM);
        return NULL;
    }

    obj->type_info = type;
    obj->type_tag  = type_tag;

    // TODO remove me when the GC mechanism is ready
    GC_REGISTER_FINALIZER ((rpointer) obj, finalize_object, state, NULL, NULL);

    return obj;
}
