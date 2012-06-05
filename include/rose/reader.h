#ifndef __ROSE_READER_H__
#define __ROSE_READER_H__

#include "rose/sexp.h"

typedef struct RReaderState RReaderState;

RReaderState* r_reader_new         (RContext*     context);
RReaderState* r_reader_from_file   (char const*   filename,
                                    RContext*     context);
RReaderState* r_reader_from_string (char const*   string,
                                    RContext*     context);
RReaderState* r_reader_from_port   (rsexp         port,  
                                    RContext*     context);
rsexp         r_read               (RReaderState* reader);
rboolean      r_reader_error_p     (RReaderState* reader);
rsexp         r_reader_last_error  (RReaderState* reader);
void          r_reader_clear_error (RReaderState* reader);

#endif  //  __ROSE_READER_H__
