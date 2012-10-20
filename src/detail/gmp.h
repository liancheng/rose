#ifndef __ROSE_DETAIL_GMP_H__
#define __ROSE_DETAIL_GMP_H__

/**
 * According to GNU MP reference manual, stdio.h should be included before
 * gmp.h if we are using GMP I/O functions like mpz_out_str.
 */

#include <stdio.h>
#include <gmp.h>

#endif  /* __ROSE_DETAIL_GMP_H__ */
