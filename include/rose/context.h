#ifndef __ROSE_CONTEXT_H__
#define __ROSE_CONTEXT_H__

typedef struct _r_context {
    void* lexer;
    void* token_queue;
}
r_context;

r_context* context_new  ();
void       context_free (r_context* context);

#endif  //  __ROSE_CONTEXT_H__
