#include "detail/compile.h"
#include "detail/env.h"
#include "detail/symbol.h"
#include "detail/state.h"
#include "detail/vm.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/pair.h"
#include "rose/io.h"
#include "rose/number.h"
#include "rose/primitive.h"
#include "rose/procedure.h"
#include "rose/sexp.h"
#include "rose/string.h"

static inline rsexp pop (RVm* vm)
{
    rsexp res = r_car (vm->stack);
    vm->stack = r_cdr (vm->stack);
    return res;
}

static inline rsexp continuation (RState* r, rsexp stack)
{
    rsexp var = r_intern_static (r, "v");
    rsexp code = emit_restore_cc (r, stack, var);
    return r_procedure_new (r, code, R_NULL, r_list (r, 1, var));
}

static inline rsexp call_frame (RState* r, rsexp ret, rsexp env,
                                rsexp args, rsexp stack)
{
    return r_list (r, 4, ret, env, args, stack);
}

static inline rsexp apply_primitive (RState* r, RVm* vm)
{
    vm->value = r_primitive_apply (r, vm->value, vm->args);
    vm->next = emit_return (r);

    return vm->value;
}

static inline rsexp extend_env (RState* r, rsexp env, rsexp formals, rsexp args)
{
    rsexp n_req;
    rsexp req_formals;
    rsexp req_args;
    rsexp rest_formal;
    rsexp rest_arg;

    if (r_null_p (formals))
        return env_extend (r, env, R_NULL, args);

    if (r_symbol_p (formals)) {
        ensure (formals = r_list (r, 1, formals));
        ensure (args = r_list (r, 1, args));
        return env_extend (r, env, formals, args);
    }

    ensure (formals = r_properfy (r, formals));

    if (r_null_p (r_cdr (formals))) {
        ensure (env = env_extend (r, env, r_car (formals), args));
        return env;
    }

    req_formals = r_car (formals);
    rest_formal = r_cdr (formals);

    ensure (n_req = r_length (r, req_formals));
    ensure (req_args = r_take (r, args, r_uint_from_sexp (n_req)));
    ensure (rest_arg = r_drop (r, args, r_uint_from_sexp (n_req)));
    ensure (env = env_extend (r, env, req_formals, req_args));
    ensure (env = r_env_bind_x (r, env, rest_formal, rest_arg));

    return env;
}

/**
 * Applies a (non-primitive) procedure.
 *
 * \pre
 *  - `vm->value` points to the procedure to be applied
 *  - `vm->args` points to the actual arguments list
 *
 * \post
 *  - A new frame is added to the current environment for the procedure call
 *  - `vm->next` points to the first instruction of the body of the  procedure
 */
static inline rsexp apply (RState* r, RVm* vm)
{
    rsexp env;
    rsexp formals;

    env = r_procedure_env (vm->value);
    formals = r_procedure_formals (vm->value);
    ensure (vm->env = extend_env (r, env, formals, vm->args));

    vm->next = r_procedure_body (vm->value);
    vm->args = R_NULL;

    return vm->value;
}

static inline rsexp exec_apply (RState* r, RVm* vm)
{
    rsexp res;

    r_gc_scope_open (r);

    if (r_procedure_p (vm->value)) {
        res = apply (r, vm);
        goto exit;
    }

    if (r_primitive_p (vm->value)) {
        res = apply_primitive (r, vm);
        goto exit;
    }

    r_error_code (r, R_ERR_WRONG_APPLY, vm->value);
    res = R_FAILURE;

exit:
    return res;
}

static inline rsexp exec_arg (RState* r, RVm* vm)
{
    rsexp args;

    ensure (args = r_cons (r, vm->value, vm->args));

    vm->args = args;
    vm->next = r_cadr (vm->next);

    return vm->value;
}

static inline rsexp exec_assign (RState* r, RVm* vm)
{
    rsexp var;
    rsexp env;

    var = r_cadr (vm->next);
    env = r_env_assign_x (r, vm->env, var, vm->value);

    if (r_failure_p (env))
        return R_FAILURE;

    vm->env = env;
    vm->next = r_caddr (vm->next);

    return vm->value;
}

static inline rsexp exec_bind (RState* r, RVm* vm)
{
    vm->env = r_env_bind_x (r, vm->env, r_cadr (vm->next), vm->value);
    vm->next = r_caddr (vm->next);

    return vm->value;
}

static inline rsexp exec_branch (RState* r, RVm* vm)
{
    vm->next = r_true_p (vm->value)
             ? r_cadr (vm->next)
             : r_caddr (vm->next);

    return vm->value;
}

static inline rsexp exec_capture_cc (RState* r, RVm* vm)
{
    vm->value = continuation (r, vm->stack);
    vm->next = r_cadr (vm->next);

    return vm->value;
}

static inline rsexp exec_constant (RState* r, RVm* vm)
{
    vm->value = r_cadr (vm->next);
    vm->next = r_caddr (vm->next);

    return vm->value;
}

static inline rsexp exec_close (RState* r, RVm* vm)
{
    rsexp formals = r_cadr (vm->next);
    rsexp body = r_caddr (vm->next);
    vm->value = r_procedure_new (r, body, vm->env, formals);
    vm->next = r_cadddr (vm->next);

    return vm->value;
}

static inline rsexp exec_frame (RState* r, RVm* vm)
{
    rsexp ret = r_cadr (vm->next);

    vm->stack = call_frame (r, ret, vm->env, vm->args, vm->stack);
    vm->args = R_NULL;
    vm->next = r_caddr (vm->next);

    return vm->value;
}

static inline rsexp exec_halt (RState* r, RVm* vm)
{
    vm->halt_p = TRUE;

    return vm->value;
}

static inline rsexp exec_refer (RState* r, RVm* vm)
{
    rsexp val = r_env_lookup (r, vm->env, r_cadr (vm->next));
    vm->value = r_undefined_p (val) ? val : r_car (val);
    vm->next = r_caddr (vm->next);

    return vm->value;
}

static inline rsexp exec_restore_cc (RState* r, RVm* vm)
{
    rsexp stack = r_cadr (vm->next);
    rsexp var = r_caddr (vm->next);

    vm->value = r_car (r_env_lookup (r, vm->env, var));
    vm->next = emit_return (r);
    vm->stack = stack;

    return vm->value;
}

static inline rsexp exec_return (RState* r, RVm* vm)
{
    vm->next = pop (vm);
    vm->env = pop (vm);
    vm->args = pop (vm);
    vm->stack = r_car (vm->stack);

    r_gc_scope_close (r);

    return vm->value;
}

static rsexp single_step (RState* r)
{
    RVm* vm;
    rsexp ins;
    rsexp res;
    rsexp (*exec) (RState*, RVm*);

    vm = &r->vm;
    ins = r_car (vm->next);

    switch (ins) {
        case R_OP_APPLY:      exec = exec_apply;      break;
        case R_OP_ARG:        exec = exec_arg;        break;
        case R_OP_ASSIGN:     exec = exec_assign;     break;
        case R_OP_CAPTURE_CC: exec = exec_capture_cc; break;
        case R_OP_CONSTANT:   exec = exec_constant;   break;
        case R_OP_CLOSE:      exec = exec_close;      break;
        case R_OP_BIND:       exec = exec_bind;       break;
        case R_OP_BRANCH:     exec = exec_branch;     break;
        case R_OP_FRAME:      exec = exec_frame;      break;
        case R_OP_HALT:       exec = exec_halt;       break;
        case R_OP_REFER:      exec = exec_refer;      break;
        case R_OP_RESTORE_CC: exec = exec_restore_cc; break;
        case R_OP_RETURN:     exec = exec_return;     break;

        default:
            r_error_code (r, R_ERR_UNKNOWN_INSTR, vm->next);
            return R_FAILURE;
    }

    r_gc_scope_open (r);
    res = exec (r, vm);
    r_gc_scope_close (r);

    return res;
}

void vm_dump (RState* r)
{
    RVm* vm = &r->vm;

    r_format (r, "(vm:args  ~s~%", vm->args);
    r_format (r, " vm:env   ~s~%", vm->env);
    r_format (r, " vm:stack ~s~%", vm->stack);
    r_format (r, " vm:value ~s~%", vm->value);
    r_format (r, " vm:next  ~s)~%", vm->next);
}

void vm_init (RState* r)
{
    RVm* vm = &r->vm;

    r_gc_scope_open (r);

    vm->args   = R_UNDEFINED;
    vm->env    = default_env (r);
    vm->stack  = R_NULL;
    vm->value  = R_UNDEFINED;
    vm->next   = R_UNDEFINED;
    vm->halt_p = FALSE;

    r_gc_scope_close (r);
}

void vm_finish (RState* r)
{
    r->vm.args  = R_UNDEFINED;
    r->vm.env   = R_UNDEFINED;
    r->vm.stack = R_UNDEFINED;
    r->vm.value = R_UNDEFINED;
    r->vm.next  = R_UNDEFINED;
}

rsexp r_eval (RState* r, rsexp code)
{
    RVm* vm = &r->vm;

    vm->next = code;
    vm->halt_p = FALSE;

    while (!vm->halt_p)
        ensure (single_step (r));

    return vm->value;
}

rsexp r_eval_from_port (RState* r, rsexp port)
{
    rsexp code;
    ensure (code = r_compile_from_port (r, port));
    return r_eval (r, code);
}

rsexp r_eval_from_string (RState* r, rsexp input)
{
    rsexp port;
    rsexp code;

    ensure (port = r_open_input_string (r, input));
    ensure (code = r_compile_from_port (r, port));

    return r_eval (r, code);
}

rsexp r_eval_from_cstr (RState* r, rconstcstring input)
{
    return r_eval_from_string (r, r_string_new (r, input));
}

rsexp r_eval_from_file (RState* r, rconstcstring path)
{
    return r_eval_from_port (r, r_open_input_file (r, path));
}
