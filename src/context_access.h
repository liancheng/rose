#ifndef __ROSE_CONTEXT_ACCESS_H__
#define __ROSE_CONTEXT_ACCESS_H__

#include "rose/context.h"
#include "rose/types.h"

#define DECLARE_CONTEXT_FIELD_GETTER(field)\
        rpointer r_context_get_##field(RContext* context);

DECLARE_CONTEXT_FIELD_GETTER(scanner);
DECLARE_CONTEXT_FIELD_GETTER(symbol_table);
DECLARE_CONTEXT_FIELD_GETTER(keywords);
DECLARE_CONTEXT_FIELD_GETTER(global_env);

#undef DECLARE_CONTEXT_FIELD_GETTER

#define CONTEXT_FIELD(field, context) r_context_get_##field(context);

#endif  //  __ROSE_CONTEXT_ACCESS_H__
