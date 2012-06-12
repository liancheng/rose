#include "detail/context.h"

#include "rose/env.h"
#include "rose/port.h"
#include "rose/symbol.h"

#include <gc/gc.h>
#include <assert.h>

RContext* r_context_new ()
{
    RContext* context = GC_NEW (RContext);

    context->symbol_table        = r_symbol_table_new ();
    context->env                 = r_env_new ();
    context->current_input_port  = r_stdin_port ();
    context->current_output_port = r_stdout_port ();

    return context;
}
