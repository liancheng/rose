/* -*- C++ -*- vim:set syntax=cpp: 
 * (C) 2010 Frank-Rene Schaefer    
 * ABSOLUTELY NO WARRANTY                    */
#ifndef __QUEX_INCLUDE_GUARD__AUX_STRING_I
#define __QUEX_INCLUDE_GUARD__AUX_STRING_I

#include <quex/code_base/definitions>

QUEX_NAMESPACE_MAIN_OPEN

QUEX_INLINE size_t 
QUEX_NAME(strlen)(const QUEX_TYPE_CHARACTER* Str)
{
    const QUEX_TYPE_CHARACTER* iterator = Str;
    while( *iterator != 0 ) ++iterator; 
    return (size_t)(iterator - Str);
}

QUEX_INLINE size_t 
QUEX_NAME(strcmp)(const QUEX_TYPE_CHARACTER* it0, 
                  const QUEX_TYPE_CHARACTER* it1)
{
    for(; *it0 == *it1; ++it0, ++it1) {
        /* Both letters are the same and == 0?
         * => both reach terminall zero without being different. */
        if( *it0 == 0 ) return 0;
    }
    return (size_t)(*it0) - (size_t)(*it1);
}

QUEX_NAMESPACE_MAIN_CLOSE

#endif /* __QUEX_INCLUDE_GUARD__AUX_STRING_I */
