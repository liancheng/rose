#ifndef __ROSE_MEMORY_H__
#define __ROSE_MEMORY_H__

#include "rose/state.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

rpointer r_alloc (RState* r, rsize size);

rpointer r_realloc (RState* r, rpointer ptr, rsize size);

rpointer r_calloc (RState* r, rsize element_size, rsize count);

void r_free (RState* r, rpointer ptr);

#define r_new(r, type)\
        ((type*) r_alloc (r, sizeof (type)))

#define r_new0(r, type)\
        ((type*) r_calloc (r, sizeof (type), 1u))

#define r_new_array(r, type, n)\
        ((type*) r_alloc (r, sizeof (type) * (n)))

#define r_new0_array(r, type, n)\
        ((type*) r_calloc (r, sizeof (type), (n)))

/**
 * Opens a GC scope.  All objects allocated within the scope are protected and
 * won't be collected by the garbage collector.
 *
 * \see r_gc_scope_close
 * \see r_gc_scope_close_and_protect
 */
void r_gc_scope_open (RState* r);

/**
 * Closes an opened GC scope.  Objects allocated within the scope are no longer
 * protected and can be collected during next GC.
 */
void r_gc_scope_close (RState* r);

/**
 * Prevents a single `rsexp` object from being garbage collected.
 *
 * \note This function must be called outside any GC scope.
 */
void r_gc_scope_protect (RState* r, rsexp obj);

/**
 * Triggers a round of full GC.
 */
void r_full_gc (RState* r);

void r_gc_mark (RState* r, rsexp obj);

#define r_gc_scope_close_and_protect(r, obj)\
        do {\
            r_gc_scope_close (r);\
            r_gc_scope_protect (r, obj);\
        }\
        while (0)

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_MEMORY_H__ */
