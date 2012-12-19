#include "utils.hpp"

#include "detail/gc.h"
#include "detail/state.h"

class test_gc_scope : public fixture_base {};

TEST_F (test_gc_scope, empty_gc_scope) {
    rsize i = state->gc.arena_index;

    r_gc_scope_open (state);
    EXPECT_EQ (i, state->gc.arena_index);
    r_gc_scope_close (state);
    EXPECT_EQ (i, state->gc.arena_index);
}
