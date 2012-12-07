#ifndef __ROSE_READER_H__
#define __ROSE_READER_H__

#include "rose/sexp.h"

R_BEGIN_DECLS

typedef struct RDatumReader RDatumReader;

RDatumReader* r_reader_new         (RState*       state);
RDatumReader* r_file_reader        (RState*       state,
                                    rconstcstring filename);
RDatumReader* r_string_reader      (RState*       state,
                                    rconstcstring string);
RDatumReader* r_port_reader        (RState*       state,
                                    rsexp         port);
void          r_reader_free        (RState*       state,
                                    RDatumReader* reader);
rsexp         r_reader_last_error  (RState*       state,
                                    RDatumReader* reader);
void          r_reader_clear_error (RState*       state,
                                    RDatumReader* reader);
rsexp         r_read               (RState*       state,
                                    RDatumReader* reader);

R_END_DECLS

#endif  /* __ROSE_READER_H__ */
