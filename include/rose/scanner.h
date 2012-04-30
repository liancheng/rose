#ifndef __ROSE_SCANNER_H__
#define __ROSE_SCANNER_H__

#include "rose/context.h"
#include "rose/scanner_types.h"

r_scanner* scanner_new           ();
void       scanner_free          (r_scanner* scanner);
void       scanner_init          (FILE*      input,
                                  r_context* context);
r_token*   scanner_next_token    (FILE*      input,
                                  r_context* context);
r_token*   scanner_peek_token    (FILE*      input,
                                  r_context* context);
r_token_id scanner_peek_token_id (FILE*      input,
                                  r_context* context);
void       scanner_consume_token (FILE*      input,
                                  r_context* context);
r_token*   scanner_copy_token    (r_token*   token);
void       scanner_free_token    (r_token*   token);

#endif  //  __ROSE_SCANNER_H__
