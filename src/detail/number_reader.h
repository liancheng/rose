#ifndef __ROSE_DETAIL_NUMBER_READER_H__
#define __ROSE_DETAIL_NUMBER_READER_H__

#include "rose/gmp.h"
#include "rose/sexp.h"
#include "rose/types.h"

R_BEGIN_DECLS

typedef struct RNumberReader RNumberReader;

struct RNumberReader {
    RState*       r;

    rconstcstring begin;
    rconstcstring end;
    rconstcstring pos;

    rtribool      exact;
    rtribool      decimal;
    ruintw        radix;
};

void  r_number_reader_init (RState* r,
                            RNumberReader* reader);
rsexp r_number_read        (RNumberReader* reader,
                            rconstcstring text);

R_END_DECLS

#endif /* __ROSE_DETAIL_NUMBER_READER_H__ */
