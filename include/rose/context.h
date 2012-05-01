#ifndef __ROSE_CONTEXT_H__
#define __ROSE_CONTEXT_H__

typedef struct RContext RContext;

RContext* r_context_new  ();
void      r_context_free (RContext* context);

#endif  //  __ROSE_CONTEXT_H__
