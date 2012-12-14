#ifndef __ROSE_READER_H__
#define __ROSE_READER_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

typedef struct RDatumReader RDatumReader;

RDatumReader* r_reader_new  (RState*       state,
                             rsexp         port);
void          r_reader_free (RDatumReader* reader);
rsexp         r_read        (RDatumReader* reader);

R_END_DECLS

#endif  /* __ROSE_READER_H__ */
