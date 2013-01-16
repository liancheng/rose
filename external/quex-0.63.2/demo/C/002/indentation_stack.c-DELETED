#include "EasyLexer"

void
IndentationStack_push(IndentationStack* me, uint16_t Indentation)
{
    if( me->end == me->memory_end ) 
        QUEX_ERROR_EXIT("Indentation stack overflow.");
    *(me->end++) = Indentation;
}

uint16_t
IndentationStack_pop(IndentationStack* me)
{
    __quex_assert( me->end != me->begin );
    return *(--(me->end));
}
