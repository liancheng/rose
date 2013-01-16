#ifndef __INCLUDE_GUARD__MESSAGING_FRAMEWORK__
#define __INCLUDE_GUARD__MESSAGING_FRAMEWORK__


#ifndef __QUEX_OPTION_MESSAGE_UTF8
#   include "tiny_lexer.h"
#   define ELEMENT_TYPE QUEX_TYPE_CHARACTER
#else
#   include "tiny_lexer_utf8.h"
#   define ELEMENT_TYPE uint8_t
#endif

/* Assume that some low level driver communicates the place where 
 * input is placed via macros.                                     */
#define  MESSAGING_FRAMEWORK_BUFFER_SIZE  ((size_t)(512))
extern ELEMENT_TYPE   MESSAGING_FRAMEWORK_BUFFER[MESSAGING_FRAMEWORK_BUFFER_SIZE];

extern size_t messaging_framework_receive(ELEMENT_TYPE** buffer);
extern size_t messaging_framework_receive_whole_characters(ELEMENT_TYPE** rx_buffer);
extern size_t messaging_framework_receive_syntax_chunk(ELEMENT_TYPE** buffer);
extern size_t messaging_framework_receive_into_buffer(ELEMENT_TYPE*, size_t);
extern size_t messaging_framework_receive_into_buffer_syntax_chunk(ELEMENT_TYPE* BufferBegin, 
                                                                   size_t BufferSize);
extern size_t messaging_framework_receive_to_internal_buffer();
extern void   messaging_framework_release(ELEMENT_TYPE*);

#endif /*_INCLUDE_GUARD__MESSAGING_FRAMEWORK_*/
