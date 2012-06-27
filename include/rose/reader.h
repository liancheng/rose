#ifndef __ROSE_READER_H__
#define __ROSE_READER_H__

#include "rose/sexp.h"

typedef struct RDatumReader RDatumReader;

RDatumReader* r_reader_new         (RContext*     context);
RDatumReader* r_reader_from_file   (char const*   filename,
                                    RContext*     context);
RDatumReader* r_reader_from_string (char const*   string,
                                    RContext*     context);
RDatumReader* r_reader_from_port   (rsexp         port,  
                                    RContext*     context);
rsexp         r_read               (RDatumReader* reader);
rboolean      r_reader_error_p     (RDatumReader* reader);
rsexp         r_reader_last_error  (RDatumReader* reader);
void          r_reader_clear_error (RDatumReader* reader);

#endif  //  __ROSE_READER_H__
