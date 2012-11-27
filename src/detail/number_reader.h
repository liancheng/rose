#ifndef __ROSE_DETAIL_NUMBER_READER_H__
#define __ROSE_DETAIL_NUMBER_READER_H__

#include "detail/gmp.h"
#include "rose/sexp.h"
#include "rose/types.h"

typedef struct RNumberReader RNumberReader;

struct RNumberReader {
    RState*       state;

    rconstcstring begin;
    rconstcstring end;
    rconstcstring pos;

    rtribool      exact;
    rtribool      decimal;
    ruint         radix;
};

RNumberReader*  r_number_reader_new (RState*        state);
rsexp           r_number_read       (RNumberReader* reader,
                                     rconstcstring  text);

#endif  /* __ROSE_DETAIL_NUMBER_READER_H__ */
