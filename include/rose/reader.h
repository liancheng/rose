#ifndef __ROSE_READER_H__
#define __ROSE_READER_H__

#include "rose/sexp.h"

typedef struct RDatumReader RDatumReader;

RDatumReader* r_reader_new         (RContext*     context);
RDatumReader* r_file_reader        (char const*   filename,
                                    RContext*     context);
RDatumReader* r_string_reader      (char const*   string,
                                    RContext*     context);
RDatumReader* r_port_reader        (rsexp         port,
                                    RContext*     context);
rsexp         r_reader_last_error  (RDatumReader* reader);
void          r_reader_clear_error (RDatumReader* reader);
rsexp         r_read               (RDatumReader* reader);

#endif  //  __ROSE_READER_H__
