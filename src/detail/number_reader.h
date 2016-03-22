#ifndef __ROSE_DETAIL_NUMBER_READER_H__
#define __ROSE_DETAIL_NUMBER_READER_H__

#include "rose/gmp.h"
#include "rose/sexp.h"
#include "rose/types.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

typedef struct RNumberReader RNumberReader;

/**
 * An LL(*) backtracking parser for reading Scheme numbers.
 */
struct RNumberReader {
    /// Interpreter state.
    RState* r;

    /// Pointer to the beginning of the input text.
    rconstcstring begin;

    /// Pointer to the end of the input text.
    rconstcstring end;

    /// Pointer to the next character to be read.
    rconstcstring pos;

    /// Whether the number is exact.
    rtribool exact;

    /// Whether the number is decimal.
    rtribool decimal;

    /// Radix of the number.
    ruintw radix;
};

void r_number_reader_init (RState* r, RNumberReader* reader);

rsexp r_number_read (RNumberReader* reader, rconstcstring text);

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_DETAIL_NUMBER_READER_H__ */
