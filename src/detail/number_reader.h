#ifndef __ROSE_DETAIL_NUMBER_READER_H__
#define __ROSE_DETAIL_NUMBER_READER_H__

#include "detail/gmp.h"
#include "rose/sexp.h"
#include "rose/types.h"

#include <setjmp.h>

typedef struct RNumberReader RNumberReader;

struct RNumberReader {
    char const* begin;
    char const* end;
    char const* pos;
    rtribool    exact;
    ruint       radix;
};

RNumberReader*  r_number_reader_new             ();
void            r_number_reader_feed_input      (RNumberReader* reader,
                                                 char const*    text);
char            r_number_reader_lookahead       (RNumberReader* reader,
                                                 ruint          n);
char            r_number_reader_next            (RNumberReader* reader);
void            r_number_reader_consume         (RNumberReader* reader,
                                                 ruint          n);
rboolean        r_number_reader_eoi_p           (RNumberReader* reader);
char const*     r_number_reader_mark            (RNumberReader* reader);
rboolean        r_number_reader_rewind          (RNumberReader* reader,
                                                 char const*    mark);

rsexp           r_number_read                   (RNumberReader* reader,
                                                 char const*    text);
rsexp           r_number_read_number            (RNumberReader* reader);
rboolean        r_number_read_prefix            (RNumberReader* reader);
rboolean        r_number_read_radix             (RNumberReader* reader);
rboolean        r_number_read_exactness         (RNumberReader* reader);
rboolean        r_number_read_polar_complex     (RNumberReader* reader,
                                                 double*        rho,
                                                 double*        theta);
rboolean        r_number_read_rect_complex      (RNumberReader* reader,
                                                 mpq_t          real,
                                                 mpq_t          imag);
rboolean        r_number_read_rect_i            (RNumberReader* reader,
                                                 mpq_t          real,
                                                 mpq_t          imag);
rboolean        r_number_read_rect_ri           (RNumberReader* reader,
                                                 mpq_t          real,
                                                 mpq_t          imag);
rboolean        r_number_read_rect_r            (RNumberReader* reader,
                                                 mpq_t          real,
                                                 mpq_t          imag);
rboolean        r_number_read_real              (RNumberReader* reader,
                                                 mpq_t          real);
rboolean        r_number_read_ureal             (RNumberReader* reader,
                                                 mpq_t          ureal);
rboolean        r_number_read_rational          (RNumberReader* reader,
                                                 mpq_t          ureal);
rboolean        r_number_read_decimal           (RNumberReader* reader,
                                                 mpq_t          ureal);
rboolean        r_number_read_decimal_frac      (RNumberReader* reader,
                                                 mpq_t          ureal);
rboolean        r_number_read_decimal_int_frac  (RNumberReader* reader,
                                                 mpq_t          ureal);
rboolean        r_number_read_decimal_uint      (RNumberReader* reader,
                                                 mpq_t          ureal);
rboolean        r_number_read_ureal_uint        (RNumberReader* reader,
                                                 mpq_t          ureal);
rboolean        r_number_read_suffix            (RNumberReader* reader,
                                                 rint*          exponent);
rboolean        r_number_read_uinteger          (RNumberReader* reader,
                                                 mpz_t          uinteger);
rboolean        r_number_read_digits            (RNumberReader* reader,
                                                 mpz_t          digits);
rboolean        r_number_read_digit             (RNumberReader* reader,
                                                 ruint*         digit);
rboolean        r_number_read_sign              (RNumberReader* reader,
                                                 rint*          sign);

#endif  //  __ROSE_DETAIL_NUMBER_READER_H__
