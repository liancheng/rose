#ifndef __ROSE_NUMBER_H__
#define __ROSE_NUMBER_H__

#include "rose/sexp.h"

typedef struct RFixnum       RFixnum;
typedef struct RFlonum       RFlonum;
typedef struct RNumberReader RNumberReader;

rboolean r_number_read      (RNumberReader* reader);
rsexp    r_string_to_number (char const*    text);

#endif  //  __ROSE_NUMBER_H__
