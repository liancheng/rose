#ifndef __ROSE_DETAIL_SEXP_H__
#define __ROSE_DETAIL_SEXP_H__

#include "rose/sexp.h"
#include "rose/state.h"

typedef enum {
    R_TYPE_STRING = R_TAG_MAX + 1,
    R_TYPE_VECTOR,
    R_TYPE_BYTEVECTOR,
    R_TYPE_PROCEDURE,
    R_TYPE_CONTINUATION,
    R_TYPE_ENV,
    R_TYPE_PORT,
    R_TYPE_ERROR,
    R_TYPE_FIXNUM,
    R_TYPE_FLONUM,
    R_TYPE_END
}
RBoxedTypeTag;

typedef struct RObject         RObject;
typedef struct RTypeDescriptor RTypeDescriptor;

#define R_OBJECT_HEADER\
        RTypeDescriptor* meta;\
        RBoxedTypeTag    tag: 8;\
        ruint            gc_color : 3;\
        RObject*         gc_next;

typedef rbool (*REqvPred)     (RState*, rsexp, rsexp);
typedef rbool (*REqualPred)   (RState*, rsexp, rsexp);
typedef void  (*RWriteFunc)   (rsexp,   rsexp);
typedef void  (*RDisplayFunc) (rsexp,   rsexp);
typedef void  (*RGcMarkFunc)  (RState*, rsexp);
typedef void  (*RObjDestruct) (RState*, RObject*);

struct RObject {
    R_OBJECT_HEADER
};

struct RTypeDescriptor {
    rsize size;
    char* name;

    struct {
        RWriteFunc   write;
        RDisplayFunc display;
        REqvPred     eqv_p;
        REqualPred   equal_p;
        RGcMarkFunc  mark;
        RObjDestruct destruct;
    }
    ops;
};

#define R_SEXP_TYPE(obj)    (*(RTypeDescriptor**) (obj))

ruint            r_type_tag       (rsexp            obj);
RTypeDescriptor* r_describe       (RState*          state,
                                   rsexp            obj);
void             r_register_types (RState*          state);
RObject*         r_object_new     (RState*          state,
                                   RBoxedTypeTag    tag,
                                   RTypeDescriptor* meta);

#endif  /* __ROSE_DETAIL_SEXP_H__ */
