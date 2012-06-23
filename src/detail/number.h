#ifndef __ROSE_DETAIL_NUMBER_H__
#define __ROSE_DETAIL_NUMBER_H__

#include "detail/gmp.h"
#include "rose/number.h"

struct RFixnum {
    mpq_t real;
    mpq_t imag;
};

struct RFlonum {
    double real;
    double imag;
};

#endif  //  __ROSE_DETAIL_NUMBER_H__
