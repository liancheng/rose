#ifndef __ROSE_DETAIL_SEXP_H__
#define __ROSE_DETAIL_SEXP_H__

#include "rose/sexp.h"
#include "rose/state.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

typedef struct RTypeInfo RTypeInfo;

typedef rbool (*REqvPred)     (RState*, rsexp, rsexp);
typedef rbool (*REqualPred)   (RState*, rsexp, rsexp);
typedef rsexp (*RWriteFunc)   (RState*, rsexp, rsexp);
typedef rsexp (*RDisplayFunc) (RState*, rsexp, rsexp);
typedef void  (*RGcMark)      (RState*, rsexp);
typedef void  (*RGcFinalize)  (RState*, RObject*);

struct RTypeInfo {
    rsize size;
    char* name;

    struct {
        RWriteFunc   write;
        RDisplayFunc display;
        REqvPred     eqv_p;
        REqualPred   equal_p;
        RGcMark      mark;
        RGcFinalize  finalize;
    }
    ops;
};

ruintw r_type_tag  (rsexp obj);

RTypeInfo* r_type_info (RState* r, rsexp obj);

#define STATIC_OBJECT_HEADER(tag)\
        .type_tag = (tag),\
        .gc_color = GC_COLOR_BLACK,\
        .gray_next = NULL,\
        .chrono_next = NULL,

#define object_from_sexp(obj)   (r_cast (RObject*, (obj)))
#define object_to_sexp(obj)     (r_cast (rsexp, (obj)))

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_DETAIL_SEXP_H__ */
