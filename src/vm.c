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
#include "rose/string.h"
#include "rose/symbol.h"

typedef rsexp (*RInstructionExecutor) (RState*, RVm*);

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

static inline rsexp call_frame (RState* r,
                         rsexp ret,
                         rsexp env,
                         rsexp args,
                         rsexp stack)
{
    return r_list (r, 4, ret, env, args, stack);
}

static inline rsexp apply_primitive (RState* r, RVm* vm)
{
    r_gc_scope_open (r);

    vm->value = r_primitive_apply (r, vm->value, vm->args);
    vm->next = emit_return (r);

    return vm->value;
}

static inline rsexp apply (RState* r, RVm* vm)
{
    rsexp proc, body, env, formals;

    proc = vm->value;
    body = r_procedure_body (proc);
    env = r_procedure_env (proc);
    formals = r_procedure_formals (proc);

    vm->next = body;
    vm->env = env_extend (r, env, formals, vm->args);
    vm->args = R_NULL;

    r_gc_scope_open (r);

    return vm->value;
}

static inline rsexp exec_apply (RState* r, RVm* vm)
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

    var = r_cadr (vm->next);
    vm->env = r_env_assign_x (r, vm->env, var, vm->value);

    if (r_failure_p (vm->env))
        return R_FAILURE;

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
    vm->args  = R_NULL;
    vm->next  = r_caddr (vm->next);

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
    vm->next  = emit_return (r);
    vm->stack = stack;

    return vm->value;
}

static inline rsexp exec_return (RState* r, RVm* vm)
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
    RVm* vm;
    rsexp ins;
    rsexp res;
    RInstructionExecutor exec;

    vm = &r->vm;
    ins = r_car (vm->next);
    exec = r_cast (RInstructionExecutor,
                   g_hash_table_lookup (vm->executors,
                                        r_cast (rconstpointer, ins)));

    if (!exec) {
        r_error_code (r, R_ERR_UNKNOWN_INSTR, vm->next);
        return R_FAILURE;
    }

    r_gc_scope_open (r);
    res = exec (r, vm);
    r_gc_scope_close (r);

    return res;
}

static void install_instruction_executor (RState* r,
                                          RVm* vm,
                                          rsexp ins,
                                          RInstructionExecutor exec)
{
    g_hash_table_insert (vm->executors,
                         r_cast (rpointer, ins),
                         r_cast (rpointer, exec));
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

    vm->args   = R_UNDEFINED;
    vm->env    = default_env (r);
    vm->stack  = R_NULL;
    vm->value  = R_UNDEFINED;
    vm->next   = R_UNDEFINED;
    vm->halt_p = FALSE;

    vm->executors = g_hash_table_new (g_direct_hash, g_direct_equal);

    install_instruction_executor (r, vm, r->i.apply,      exec_apply);
    install_instruction_executor (r, vm, r->i.arg,        exec_arg);
    install_instruction_executor (r, vm, r->i.assign,     exec_assign);
    install_instruction_executor (r, vm, r->i.capture_cc, exec_capture_cc);
    install_instruction_executor (r, vm, r->i.constant,   exec_constant);
    install_instruction_executor (r, vm, r->i.close,      exec_close);
    install_instruction_executor (r, vm, r->i.bind,       exec_bind);
    install_instruction_executor (r, vm, r->i.branch,     exec_branch);
    install_instruction_executor (r, vm, r->i.frame,      exec_frame);
    install_instruction_executor (r, vm, r->i.halt,       exec_halt);
    install_instruction_executor (r, vm, r->i.refer,      exec_refer);
    install_instruction_executor (r, vm, r->i.restore_cc, exec_restore_cc);
    install_instruction_executor (r, vm, r->i.return_,    exec_return);
}

void vm_finish (RState* r)
{
    r->vm.args  = R_UNDEFINED;
    r->vm.env   = R_UNDEFINED;
    r->vm.stack = R_UNDEFINED;
    r->vm.value = R_UNDEFINED;
    r->vm.next  = R_UNDEFINED;

    g_hash_table_destroy (r->vm.executors);
}

rsexp r_eval (RState* r, rsexp code)
{
    RVm* vm = &r->vm;

    vm->next = code;
    vm->halt_p = FALSE;

    while (!vm->halt_p)
        if (r_failure_p (single_step (r)))
            return R_FAILURE;

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
