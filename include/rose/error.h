#ifndef __ROSE_ERROR_H__
#define __ROSE_ERROR_H__

#include "rose/sexp.h"
#include "rose/state.h"

#include <stdarg.h>
#include <setjmp.h>

R_BEGIN_DECLS

typedef enum {
    R_ERR_GRP_COMPILE,
    R_ERR_GRP_RUNTIME,
    R_ERR_GRP_MISC
}
RErrorGroup;

typedef enum {
#define ERROR_DESC(group, enum_name, desc) enum_name,
#include "rose/error_desc.inc"
#undef ERROR_DESC
}
RErrorCode;

typedef struct RErrorDesc RErrorDesc;

struct RErrorDesc {
    RErrorGroup group;
    RErrorCode code;
    rconstcstring desc;
};

rsexp r_error_new (RState* r, rsexp message, rsexp irritants);

rbool r_error_p (rsexp obj);

rsexp r_error_object_message (rsexp error);

rsexp r_error_object_irritants (rsexp error);

rsexp r_error_printf (RState* r, rconstcstring format, ...);

rsexp r_error_format (RState* r, rconstcstring format, ...);

rsexp r_error (RState* r, rconstcstring message);

rsexp r_error_code (RState* r, RErrorCode error_code, ...);

rsexp r_last_error (RState* r);

rsexp r_set_last_error_x (RState* r, rsexp error);

rsexp r_clear_last_error_x (RState* r);

rsexp r_inherit_errno_x (RState* r, rintw errnum);

void r_error_no_memory (RState* r);

typedef struct RNestedJump RNestedJump;

struct RNestedJump {
    RNestedJump* previous;
    jmp_buf      buf;
};

void r_raise (RState* r);

#define ensure(stmt)\
        do {\
            if (r_failure_p (stmt))\
                return R_FAILURE;\
        }\
        while (0)

#define ensure_or_goto(stmt, label)\
        do {\
            if (r_failure_p (stmt))\
                goto label;\
        }\
        while (0)

#define ensure_or_ret(stmt, ret)\
        do {\
            if (r_failure_p (stmt))\
                return (ret);\
        }\
        while (0)

R_END_DECLS

#endif /* __ROSE_ERROR_H__ */
