#ifndef __ROSE_READER_H__
#define __ROSE_READER_H__

#include "rose/sexp.h"

typedef struct RDatumReader RDatumReader;

RDatumReader* r_reader_new         (RState*       state);
RDatumReader* r_file_reader        (RState*       state,
                                    char const*   filename);
RDatumReader* r_string_reader      (RState*       state,
                                    char const*   string);
RDatumReader* r_port_reader        (RState*       state,
                                    rsexp         port);
void          r_reader_free        (RState*       state,
                                    RDatumReader* reader);
rsexp         r_reader_last_error  (RDatumReader* reader);
void          r_reader_clear_error (RDatumReader* reader);
rsexp         r_read               (RDatumReader* reader);

#endif  /* __ROSE_READER_H__ */
