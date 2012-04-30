#ifndef __ROSE_SCANNER_H__
#define __ROSE_SCANNER_H__

#include "rose/context.h"
#include "rose/scanner_types.h"

RScanner* r_scanner_new           ();
void      r_scanner_free          (RScanner* scanner);
void      r_scanner_init          (FILE*     input,
                                   RContext* context);
RToken*   r_scanner_next_token    (FILE*     input,
                                   RContext* context);
RToken*   r_scanner_peek_token    (FILE*     input,
                                   RContext* context);
rtokenid  r_scanner_peek_token_id (FILE*     input,
                                   RContext* context);
void      r_scanner_consume_token (FILE*     input,
                                   RContext* context);
RToken*   r_scanner_copy_token    (RToken*   token);
void      r_scanner_free_token    (RToken*   token);

#endif  //  __ROSE_SCANNER_H__
