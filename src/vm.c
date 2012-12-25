#include "detail/closure.h"
#include "detail/env.h"
#include "detail/vm.h"
#include "detail/state.h"
#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/sexp.h"

typedef void (*RInstructionExecutor) (RState*, RVm*);

static void exec_apply (RState* state, RVm* vm)
{
    rsexp proc      = vm->value;
    rsexp proc_env  = r_closure_env (proc);
    rsexp proc_vars = r_closure_vars (proc);

    vm->next = r_closure_body (proc);
    vm->env  = extend (state, proc_env, proc_vars, vm->args);
    vm->args = R_NULL;
}

static void exec_arg (RState* state, RVm* vm)
{
    vm->args = r_cons (state, vm->value, vm->args);
    vm->next = r_cadr (vm->next);
}

static void exec_assign (RState* state, RVm* vm)
{
    vm->env = assign_x (state, vm->env, r_cadr (vm->next), vm->value);
    vm->next = r_caddr (vm->next);
}

static void exec_bind (RState* state, RVm* vm)
{
    vm->env = bind_x (state, vm->env, r_cadr (vm->next), vm->value);
    vm->next = r_caddr (vm->next);
}

static void exec_branch (RState* state, RVm* vm)
{
    vm->next = r_true_p (vm->value)
             ? r_cadr (vm->next)
             : r_caddr (vm->next);
}

static void exec_const (RState* state, RVm* vm)
{
    vm->value = r_cadr (vm->next);
    vm->next = r_caddr (vm->next);
}

static void exec_close (RState* state, RVm* vm)
{
    rsexp vars = r_cadr (vm->next);
    rsexp body = r_caddr (vm->next);
    vm->value = r_closure_new (state, body, vm->env, vars);
    vm->next = r_cadddr (vm->next);
}

static void exec_frame (RState* state, RVm* vm)
{
    rsexp ret = r_cadr (vm->next);
    vm->stack = r_list (state, 4, ret, vm->env, vm->args, vm->stack);
    vm->args = R_NULL;
    vm->next = r_caddr (vm->next);
}

static void exec_halt (RState* state, RVm* vm)
{
    vm->halt_p = TRUE;
}

static void exec_refer (RState* state, RVm* vm)
{
    rsexp val = lookup (state, vm->env, r_cadr (vm->next));
    vm->value = r_undefined_p (val) ? val : r_car (val);
    vm->next = r_caddr (vm->next);
}

static rsexp pop (RVm* vm)
{
    rsexp res = r_car (vm->stack);
    vm->stack = r_cdr (vm->stack);
    return res;
}

static void exec_return (RState* state, RVm* vm)
{
    vm->next  = pop (vm);
    vm->env   = pop (vm);
    vm->args  = pop (vm);
    vm->stack = r_car (vm->stack);
}

static void single_step (RState* state)
{
    RVm* vm = &state->vm;
    rsexp ins = r_car (vm->next);
    RInstructionExecutor exec;

    exec = r_cast (RInstructionExecutor,
                   g_hash_table_lookup (vm->executors,
                                        r_cast (rconstpointer, ins)));

    if (exec)
        exec (state, &state->vm);
}

static void install_instruction_executor (RState*              state,
                                          RVm*                 vm,
                                          RReservedWord        ins,
                                          RInstructionExecutor exec)
{
    g_hash_table_insert (vm->executors,
                         r_cast (rpointer, reserved (state, ins)),
                         r_cast (rpointer, exec));
}

static void vm_dump (RState* state)
{
    RVm* vm = &state->vm;

    r_format (state, "*** vm dump ***~%");
    r_format (state, "vm.args  = ~s~%", vm->args);
    r_format (state, "vm.env   = ~s~%", vm->env);
    r_format (state, "vm.stack = ~s~%", vm->stack);
    r_format (state, "vm.value = ~s~%", vm->value);
    r_format (state, "vm.next  = ~s~%", vm->next);
}

void vm_init (RState* state)
{
    RVm* vm = &state->vm;

    vm->args   = R_UNDEFINED;
    vm->env    = empty_env (state);
    vm->stack  = R_NULL;
    vm->value  = R_UNDEFINED;
    vm->next   = R_UNDEFINED;
    vm->halt_p = FALSE;

    vm->executors = g_hash_table_new (g_direct_hash, g_direct_equal);

    install_instruction_executor (state, vm, INS_APPLY,  exec_apply);
    install_instruction_executor (state, vm, INS_ARG,    exec_arg);
    install_instruction_executor (state, vm, INS_ASSIGN, exec_assign);
    install_instruction_executor (state, vm, INS_CONST,  exec_const);
    install_instruction_executor (state, vm, INS_CLOSE,  exec_close);
    install_instruction_executor (state, vm, INS_BIND,   exec_bind);
    install_instruction_executor (state, vm, INS_BRANCH, exec_branch);
    install_instruction_executor (state, vm, INS_FRAME,  exec_frame);
    install_instruction_executor (state, vm, INS_HALT,   exec_halt);
    install_instruction_executor (state, vm, INS_REFER,  exec_refer);
    install_instruction_executor (state, vm, INS_RETURN, exec_return);
}

void vm_finish (RState* state)
{
    g_hash_table_destroy (state->vm.executors);
}

rsexp r_run (RState* state, rsexp code)
{
    RVm* vm = &state->vm;

    vm->next = code;
    vm->halt_p = FALSE;

    while (!vm->halt_p) {
        vm_dump (state);
        single_step (state);
    }

    return vm->value;
}
