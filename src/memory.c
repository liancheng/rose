#include "detail/state.h"
#include "rose/memory.h"

#include <string.h>

rpointer r_realloc (RState* state, rpointer ptr, rsize size)
{
    return state->alloc_fn (state, ptr, size, state->alloc_aux);
}

rpointer r_alloc (RState* state, rsize size)
{
    return r_realloc (state, NULL, size);
}

rpointer r_calloc (RState* state, rsize element_size, rsize count)
{
    rpointer ptr = r_alloc (state, element_size * count);
    memset (ptr, 0, element_size * count);
    return ptr;
}

void r_free (RState* state, rpointer ptr)
{
    state->alloc_fn (state, ptr, 0u, state->alloc_aux);
}
