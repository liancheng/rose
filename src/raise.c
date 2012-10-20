#include "detail/raise.h"
#include "detail/state.h"

#include <stdlib.h>

void r_raise (RState* state)
{
    if (state->error_jmp) {
        longjmp (state->error_jmp->buf, 1);
    }
    else {
        abort ();
    }
}
