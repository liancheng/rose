/* -*- C++ -*- vim: set syntax=cpp: */
#ifndef __QUEX_INCLUDE_GUARD__MEMORY_MANAGER_I
#define __QUEX_INCLUDE_GUARD__MEMORY_MANAGER_I

#include <quex/code_base/definitions>

#if   defined(__QUEX_OPTION_CONVERTER)
#   include <quex/code_base/buffer/converter/BufferFiller_Converter>
#   if defined (QUEX_OPTION_CONVERTER_ICU)
#      include <quex/code_base/buffer/converter/icu/Converter_ICU>
#   endif
#   if defined (QUEX_OPTION_CONVERTER_ICONV)
#      include <quex/code_base/buffer/converter/iconv/Converter_IConv>
#   endif
#else
#   include <quex/code_base/buffer/plain/BufferFiller_Plain>
#endif

#include <quex/code_base/temporary_macros_on>
 
QUEX_NAMESPACE_LEXEME_NULL_OPEN
/* Required for unit tests on buffer and buffer filling. */
extern QUEX_TYPE_CHARACTER QUEX_LEXEME_NULL_IN_ITS_NAMESPACE; 
QUEX_NAMESPACE_LEXEME_NULL_CLOSE

QUEX_NAMESPACE_MAIN_OPEN

    QUEX_INLINE uint8_t*
    QUEX_NAME(MemoryManager_Default_allocate)(const size_t ByteN)
    {
         uint8_t*  result = (uint8_t*)__QUEX_STD_malloc((size_t)ByteN);
#        ifdef QUEX_OPTION_ASSERTS
         __QUEX_STD_memset((void*)result, 0xFF, ByteN);
#        endif
         return result;
    }
       
    QUEX_INLINE void 
    QUEX_NAME(MemoryManager_Default_free)(void* Obj)  
    { __QUEX_STD_free(Obj); }

    struct __QuexBufferFiller_tag;

    /* CONCEPT: -- All allocator functions receive an argument 'ByteN' that indicates
     *             the number of required bytes. 
     *          -- All allocator functions return a pointer to the allocated memory
     *             or '0x0' in case of failure.
     *
     *          By means of the name of the function only the 'place' of the memory
     *          might be determined easier, or an according buffer-pool strategy might
     *          be applied.                                                              */
    QUEX_INLINE QUEX_TYPE_CHARACTER*
    QUEX_NAME(MemoryManager_BufferMemory_allocate)(const size_t ByteN)
    { return (QUEX_TYPE_CHARACTER*)QUEX_NAME(MemoryManager_Default_allocate)(ByteN); }

    QUEX_INLINE void
    QUEX_NAME(MemoryManager_BufferMemory_free)(QUEX_TYPE_CHARACTER* memory)
    { if( memory != 0x0 ) QUEX_NAME(MemoryManager_Default_free)((void*)memory); }

    QUEX_INLINE void*
    QUEX_NAME(MemoryManager_BufferFiller_allocate)(const size_t ByteN)
    { return QUEX_NAME(MemoryManager_Default_allocate)(ByteN); }

    QUEX_INLINE void
    QUEX_NAME(MemoryManager_BufferFiller_free)(void* memory)
    { if( memory != 0x0 ) QUEX_NAME(MemoryManager_Default_free)((void*)memory); }

    QUEX_INLINE uint8_t*
    QUEX_NAME(MemoryManager_BufferFiller_RawBuffer_allocate)(const size_t ByteN)
    { return QUEX_NAME(MemoryManager_Default_allocate)(ByteN); }

    QUEX_INLINE void
    QUEX_NAME(MemoryManager_BufferFiller_RawBuffer_free)(uint8_t* memory)
    { if( memory != 0x0 ) QUEX_NAME(MemoryManager_Default_free)(memory); }

#if defined(__QUEX_OPTION_CONVERTER)
    QUEX_INLINE void*
    QUEX_NAME(MemoryManager_Converter_allocate)(const size_t ByteN)
    { return QUEX_NAME(MemoryManager_Default_allocate)(ByteN); }

    QUEX_INLINE void
    QUEX_NAME(MemoryManager_Converter_free)(void* memory)
    { if( memory != 0x0 ) QUEX_NAME(MemoryManager_Default_free)((void*)memory); }
#   endif

#   ifdef QUEX_OPTION_STRING_ACCUMULATOR
    QUEX_INLINE QUEX_TYPE_CHARACTER*
    QUEX_NAME(MemoryManager_Text_allocate)(const size_t ByteN)
    { return (QUEX_TYPE_CHARACTER*)QUEX_NAME(MemoryManager_Default_allocate)(ByteN); }

    QUEX_INLINE void
    QUEX_NAME(MemoryManager_Text_free)(QUEX_TYPE_CHARACTER* memory)
    { 
        /* The de-allocator shall never be called for the fix LexemeNull object. */
        __quex_assert( memory != &(QUEX_LEXEME_NULL) );

        if( memory != 0x0 ) {
            QUEX_NAME(MemoryManager_Default_free)((void*)memory); 
        }
    }
#   endif

#   ifdef QUEX_OPTION_POST_CATEGORIZER
    QUEX_INLINE  QUEX_NAME(DictionaryNode)*  
    QUEX_NAME(MemoryManager_PostCategorizerNode_allocate)(size_t RemainderL)
    {
        /* Allocate in one beat: base and remainder: 
         *
         *   [Base   |Remainder             ]
         *
         * Then bend the base->name_remainder to the Remainder part of the allocated
         * memory. Note, that this is not very efficient, since one should try to allocate
         * the small node objects and refer to the remainder only when necessary. This
         * would reduce cache misses.                                                      */
        const size_t   BaseSize      = sizeof(QUEX_NAME(DictionaryNode));
        /* Length + 1 == memory size (terminating zero) */
        const size_t   RemainderSize = sizeof(QUEX_TYPE_CHARACTER) * (RemainderL + 1);
        uint8_t*       base          = QUEX_NAME(MemoryManager_Default_allocate)(BaseSize + RemainderSize);
        ((QUEX_NAME(DictionaryNode)*)base)->name_remainder = (const QUEX_TYPE_CHARACTER*)(base + BaseSize);
        return (QUEX_NAME(DictionaryNode)*)base;
    }

    QUEX_INLINE  void 
    QUEX_NAME(MemoryManager_PostCategorizerNode_free)(QUEX_NAME(DictionaryNode)* node)
    { if( node != 0x0 ) QUEX_NAME(MemoryManager_Default_free)((void*)node); }
#   endif

    QUEX_INLINE size_t
    QUEX_NAME(MemoryManager_insert)(uint8_t* drain_begin_p,  uint8_t* drain_end_p,
                                    uint8_t* source_begin_p, uint8_t* source_end_p)
        /* Inserts as many bytes as possible into the array from 'drain_begin_p'
         * to 'drain_end_p'. The source of bytes starts at 'source_begin_p' and
         * ends at 'source_end_p'.
         *
         * RETURNS: Number of bytes that have been copied.                      */
    {
        /* Determine the insertion size. */
        const size_t DrainSize = (size_t)(drain_end_p  - drain_begin_p);
        size_t       size      = (size_t)(source_end_p - source_begin_p);
        __quex_assert(drain_end_p  >= drain_begin_p);
        __quex_assert(source_end_p >= source_begin_p);

        if( DrainSize < size ) size = DrainSize;

        /* memcpy() might fail if the source and drain domain overlap! */
#       ifdef QUEX_OPTION_ASSERTS 
        if( drain_begin_p > source_begin_p ) __quex_assert(drain_begin_p >= source_begin_p + size);
        else                                 __quex_assert(drain_begin_p <= source_begin_p - size);
#       endif
        __QUEX_STD_memcpy(drain_begin_p, source_begin_p, size);

        return size;
    }

#if defined(QUEX_OPTION_TOKEN_POLICY_QUEUE)
    QUEX_INLINE void* 
    QUEX_NAME(MemoryManager_TokenArray_allocate)(const size_t ByteN)
    { return QUEX_NAME(MemoryManager_Default_allocate)(ByteN); }

    QUEX_INLINE void 
    QUEX_NAME(MemoryManager_TokenArray_free)(void* memory)
    { if( memory != 0x0 ) QUEX_NAME(MemoryManager_Default_free)((void*)memory); }
#endif

#if defined(QUEX_OPTION_INCLUDE_STACK)
    QUEX_INLINE QUEX_NAME(Memento)*
    QUEX_NAME(MemoryManager_Memento_allocate)()
    {
        const size_t     MemorySize = sizeof(QUEX_NAME(Memento));
        return (QUEX_NAME(Memento)*)QUEX_NAME(MemoryManager_Default_allocate)(MemorySize);
    }

    QUEX_INLINE void
    QUEX_NAME(MemoryManager_Memento_free)(struct QUEX_NAME(Memento_tag)* memory)
    { if( memory != 0x0 ) QUEX_NAME(MemoryManager_Default_free)((void*)memory); }
#endif

QUEX_NAMESPACE_MAIN_CLOSE
 
#include <quex/code_base/temporary_macros_off>

#endif /*  __QUEX_INCLUDE_GUARD__MEMORY_MANAGER_I */

