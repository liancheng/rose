#include "detail/compile.h"
#include "detail/env.h"
#include "detail/vm.h"
#include "detail/state.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/primitive.h"
#include "rose/procedure.h"
#include "rose/sexp.h"
#include "rose/symbol.h"

typedef rsexp (*RInstructionExecutor) (RState*, RVm*);

static rsexp pop (RVm* vm)
{
    rsexp res = r_car (vm->stack);
    vm->stack = r_cdr (vm->stack);
    return res;
}

static rsexp continuation (RState* r, rsexp stack)
{
    rsexp var = r_symbol_new_static (r, "v");
    rsexp code = emit_restore_cc (r, stack, var);
    return r_procedure_new (r, code, R_NULL, r_list (r, 1, var));
}

static rsexp call_frame (RState* r,
                         rsexp ret,
                         rsexp env,
                         rsexp args,
                         rsexp stack)
{
    return r_list (r, 4, ret, env, args, stack);
}

static rsexp apply_primitive (RState* r, RVm* vm)
{
    r_gc_scope_open (r);

    vm->value = r_primitive_apply (r, vm->value, vm->args);
    vm->next = emit_return (r);

    return vm->value;
}

static rsexp apply (RState* r, RVm* vm)
{
    rsexp proc, body, env, formals;

    proc = vm->value;
    body = r_procedure_body (proc);
    env = r_procedure_env (proc);
    formals = r_procedure_formals (proc);

    vm->next = r_procedure_body (proc);
    vm->env = env_extend (r, env, formals, vm->args);
    vm->args = R_NULL;

    r_gc_scope_open (r);

    return vm->value;
}

static rsexp exec_apply (RState* r, RVm* vm)
{
    rsexp res;

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

static rsexp exec_arg (RState* r, RVm* vm)
{
    vm->args = r_cons (r, vm->value, vm->args);
    vm->next = r_cadr (vm->next);

    return vm->value;
}

static rsexp exec_assign (RState* r, RVm* vm)
{
    rsexp var;

    var = r_cadr (vm->next);
    vm->env = r_env_assign_x (r, vm->env, var, vm->value);

    if (r_failure_p (vm->env))
        return R_FAILURE;

    vm->next = r_caddr (vm->next);

    return vm->value;
}

static rsexp exec_bind (RState* r, RVm* vm)
{
    vm->env = r_env_bind_x (r, vm->env, r_cadr (vm->next), vm->value);
    vm->next = r_caddr (vm->next);

    return vm->value;
}

static rsexp exec_branch (RState* r, RVm* vm)
{
    vm->next = r_true_p (vm->value)
             ? r_cadr (vm->next)
             : r_caddr (vm->next);

    return vm->value;
}

static rsexp exec_capture_cc (RState* r, RVm* vm)
{
    vm->value = continuation (r, vm->stack);
    vm->next = r_cadr (vm->next);

    return vm->value;
}

static rsexp exec_const (RState* r, RVm* vm)
{
    vm->value = r_cadr (vm->next);
    vm->next = r_caddr (vm->next);

    return vm->value;
}

static rsexp exec_close (RState* r, RVm* vm)
{
    rsexp formals = r_cadr (vm->next);
    rsexp body = r_caddr (vm->next);
    vm->value = r_procedure_new (r, body, vm->env, formals);
    vm->next = r_cadddr (vm->next);

    return vm->value;
}

static rsexp exec_frame (RState* r, RVm* vm)
{
    rsexp ret = r_cadr (vm->next);

    vm->stack = call_frame (r, ret, vm->env, vm->args, vm->stack);
    vm->args  = R_NULL;
    vm->next  = r_caddr (vm->next);

    return vm->value;
}

static rsexp exec_halt (RState* r, RVm* vm)
{
    vm->halt_p = TRUE;

    return vm->value;
}

static rsexp exec_refer (RState* r, RVm* vm)
{
    rsexp val = r_env_lookup (r, vm->env, r_cadr (vm->next));
    vm->value = r_undefined_p (val) ? val : r_car (val);
    vm->next = r_caddr (vm->next);

    return vm->value;
}

static rsexp exec_restore_cc (RState* r, RVm* vm)
{
    rsexp stack = r_cadr (vm->next);
    rsexp var = r_caddr (vm->next);

    vm->value = r_car (r_env_lookup (r, vm->env, var));
    vm->next  = emit_return (r);
    vm->stack = stack;

    return vm->value;
}

static rsexp exec_return (RState* r, RVm* vm)
{
    vm->next  = pop (vm);
    vm->env   = pop (vm);
    vm->args  = pop (vm);
    vm->stack = r_car (vm->stack);

    r_gc_scope_close (r);

    return vm->value;
}

static rsexp single_step (RState* r)
{
    RVm* vm = &r->vm;
    rsexp ins = r_car (vm->next);
    RInstructionExecutor exec;

    exec = r_cast (RInstructionExecutor,
                   g_hash_table_lookup (vm->executors,
                                        r_cast (rconstpointer, ins)));

    if (!exec) {
        r_error_code (r, R_ERR_UNKNOWN_INSTR, vm->next);
        return R_FAILURE;
    }

    return exec (r, &r->vm);
}

static void install_instruction_executor (RState* r,
                                          RVm* vm,
                                          RReservedWord ins,
                                          RInstructionExecutor exec)
{
    g_hash_table_insert (vm->executors,
                         r_cast (rpointer, reserved (r, ins)),
                         r_cast (rpointer, exec));
}

void vm_dump (RState* r)
{
    RVm* vm = &r->vm;

    r_format (r, "*** vm dump ***~%");
    r_format (r, "vm.args  = ~s~%", vm->args);
    r_format (r, "vm.env   = ~s~%", vm->env);
    r_format (r, "vm.stack = ~s~%", vm->stack);
    r_format (r, "vm.value = ~s~%", vm->value);
    r_format (r, "vm.next  = ~s~%", vm->next);
}

void vm_init (RState* r)
{
    RVm* vm = &r->vm;

    vm->args   = R_UNDEFINED;
    vm->env    = default_env (r);
    vm->stack  = R_NULL;
    vm->value  = R_UNDEFINED;
    vm->next   = R_UNDEFINED;
    vm->halt_p = FALSE;

    vm->executors = g_hash_table_new (g_direct_hash, g_direct_equal);

    install_instruction_executor (r, vm, INS_APPLY,      exec_apply);
    install_instruction_executor (r, vm, INS_ARG,        exec_arg);
    install_instruction_executor (r, vm, INS_ASSIGN,     exec_assign);
    install_instruction_executor (r, vm, INS_CAPTURE_CC, exec_capture_cc);
    install_instruction_executor (r, vm, INS_CONST,      exec_const);
    install_instruction_executor (r, vm, INS_CLOSE,      exec_close);
    install_instruction_executor (r, vm, INS_BIND,       exec_bind);
    install_instruction_executor (r, vm, INS_BRANCH,     exec_branch);
    install_instruction_executor (r, vm, INS_FRAME,      exec_frame);
    install_instruction_executor (r, vm, INS_HALT,       exec_halt);
    install_instruction_executor (r, vm, INS_REFER,      exec_refer);
    install_instruction_executor (r, vm, INS_RESTORE_CC, exec_restore_cc);
    install_instruction_executor (r, vm, INS_RETURN,     exec_return);
}

void vm_finish (RState* r)
{
    g_hash_table_destroy (r->vm.executors);
}

rsexp r_run (RState* r, rsexp code)
{
    RVm* vm = &r->vm;

    vm->next = code;
    vm->halt_p = FALSE;

    while (!vm->halt_p)
        if (r_failure_p (single_step (r)))
            return R_FAILURE;

    return vm->value;
}
