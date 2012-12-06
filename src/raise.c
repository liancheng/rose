#include "rose/error.h"
#include "detail/raise.h"
#include "detail/state.h"

#include <assert.h>
#include <stdlib.h>

void r_raise (RState* state, rsexp obj)
{
    assert (r_error_p (obj));
    state->last_error = obj;

    if (state->error_jmp) {
        longjmp (state->error_jmp->buf, 1);
    }
    else {
        abort ();
    }
}
