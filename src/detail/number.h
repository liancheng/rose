#ifndef __ROSE_DETAIL_NUMBER_H__
#define __ROSE_DETAIL_NUMBER_H__

#include "rose/number.h"
#include "detail/sexp.h"

#include <gmp.h>
#include <setjmp.h>

/**
 * LL(2) parser for Scheme number.
 */
struct RNumberReader {
    char const* begin;
    char const* end;
    char const* pos;

    char        lookahead [2];
    rint        lookahead_index;

    rboolean    exact;
    rint        radix;
    rboolean    seen_exact;
    rboolean    seen_radix;

    jmp_buf     error_jmp;
};

struct RFixnum {
    mpq_t  real;
    mpq_t  imag;
};

struct RFlonum {
    mpf_t  real;
    mpf_t  imag;
};

RNumberReader* r_number_reader_new       ();
void           r_number_reader_set_input (RNumberReader* reader,
                                          char const*    begin,
                                          char const*    end);
rboolean       r_number_eoi_p            (RNumberReader* reader);
rboolean       r_number_read_exactness   (RNumberReader* reader);
rboolean       r_number_read_radix       (RNumberReader* reader);
rboolean       r_number_read_prefix      (RNumberReader* reader);
rboolean       r_number_read_complex     (RNumberReader* reader);
char           r_number_lookahead        (RNumberReader* reader,
                                          rint           k);

#endif  //  __ROSE_DETAIL_NUMBER_H__
