#ifndef __ROSE_DETAIL_SEXP_H__
#define __ROSE_DETAIL_SEXP_H__

#include "rose/sexp.h"
#include "rose/state.h"

typedef struct RObject   RObject;
typedef struct RTypeInfo RTypeInfo;

#define R_OBJECT_HEADER\
        RTypeTag type_tag : 5;\
        ruint    gc_color : 2;\
        RObject* gray_next;\
        RObject* chrono_next;

typedef rbool (*REqvPred)     (RState*, rsexp, rsexp);
typedef rbool (*REqualPred)   (RState*, rsexp, rsexp);
typedef rsexp (*RWriteFunc)   (RState*, rsexp, rsexp);
typedef rsexp (*RDisplayFunc) (RState*, rsexp, rsexp);
typedef void  (*RGcMarkFunc)  (RState*, rsexp);
typedef void  (*RFinalizer)   (RState*, RObject*);

struct RObject {
    R_OBJECT_HEADER
};

struct RTypeInfo {
    rsize size;
    char* name;

    struct {
        RWriteFunc   write;
        RDisplayFunc display;
        REqvPred     eqv_p;
        REqualPred   equal_p;
        RGcMarkFunc  mark;
        RFinalizer   finalize;
    }
    ops;
};

ruint      r_type_tag  (rsexp    obj);
RTypeInfo* r_type_info (RState*  state,
                        rsexp    obj);

#define object_from_sexp(obj)   (r_cast (RObject*, (obj)))
#define object_to_sexp(obj)     (r_cast (rsexp, (obj)))

#endif  /* __ROSE_DETAIL_SEXP_H__ */
