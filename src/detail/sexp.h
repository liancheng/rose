#ifndef __ROSE_DETAIL_SEXP_H__
#define __ROSE_DETAIL_SEXP_H__

#include "rose/sexp.h"
#include "rose/state.h"

typedef struct RObject   RObject;
typedef struct RTypeInfo RTypeInfo;

#define R_OBJECT_HEADER\
        RTypeInfo* type_info;\
        RTypeTag   type_tag : 8;\
        ruint      gc_color : 3;\
        RObject*   gc_next;

typedef rbool (*REqvPred)     (RState*, rsexp, rsexp);
typedef rbool (*REqualPred)   (RState*, rsexp, rsexp);
typedef void  (*RWriteFunc)   (RState*, rsexp, rsexp);
typedef void  (*RDisplayFunc) (RState*, rsexp, rsexp);
typedef void  (*RGcMarkFunc)  (RState*, rsexp);
typedef void  (*RObjDestruct) (RState*, RObject*);

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
        RObjDestruct destruct;
    }
    ops;
};

#define r_get_type_info(obj)    (*(RTypeInfo**) (obj))

ruint      r_type_tag     (rsexp    obj);
RTypeInfo* r_type_info    (RState*  state,
                           rsexp    obj);
RObject*   r_object_alloc (RState*  state,
                           RTypeTag type_tag);

#define r_object_new(state, type, tag)\
        (r_cast (type*, r_object_alloc (state, tag)))

void init_bool_type_info          (RState* state);
void init_char_type_info          (RState* state);
void init_special_const_type_info (RState* state);
void init_smi_type_info           (RState* state);

#endif  /* __ROSE_DETAIL_SEXP_H__ */
