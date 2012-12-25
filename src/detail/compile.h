#ifndef __ROSE_DETAIL_COMPILE_H__
#define __ROSE_DETAIL_COMPILE_H__

#include "detail/state.h"
#include "rose/pair.h"

#define emit_apply(state)\
        r_list ((state), 1,\
                reserved ((state), INS_APPLY))

#define emit_arg(state, next)\
        r_list ((state), 2,\
                reserved ((state), INS_ARG), (next))

#define emit_assign(state, var, next)\
        r_list ((state), 3,\
                reserved ((state), INS_ASSIGN), (var), (next))

#define emit_bind(state, var, next)\
        r_list ((state), 3,\
                reserved ((state), INS_BIND), (var), (next))

#define emit_branch(state, then_c, else_c)\
        r_list ((state), 3,\
                reserved ((state), INS_BRANCH), (then_c), (else_c))

#define emit_capture_cc(state, next)\
        r_list ((state), 2,\
                reserved ((state), INS_CAPTURE_CC), (next))

#define emit_close(state, vars, body, next)\
        r_list ((state), 4,\
                reserved ((state), INS_CLOSE), (vars), (body), (next))

#define emit_const(state, datum, next)\
        r_list ((state), 3,\
                reserved ((state), INS_CONST), (datum), (next))

#define emit_frame(state, next, ret)\
        r_list ((state), 3,\
                reserved ((state), INS_FRAME), (next), (ret))

#define emit_halt(state)\
        r_list ((state), 1,\
                reserved ((state), INS_HALT))

#define emit_refer(state, var, next)\
        r_list ((state), 3,\
                reserved ((state), INS_REFER), (var), (next))

#define emit_restore_cc(state, stack, var)\
        r_list ((state), 3,\
                reserved ((state), INS_RESTORE_CC), (stack), (var))

#define emit_return(state)\
        r_list ((state), 1,\
                reserved ((state), INS_RETURN))

#endif  //  __ROSE_DETAIL_COMPILE_H__
