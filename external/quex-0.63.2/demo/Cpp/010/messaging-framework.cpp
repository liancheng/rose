#include <cstdlib>
#include <cstring>
#include <assert.h>
#include "messaging-framework.h"

#ifndef __QUEX_OPTION_MESSAGE_UTF8
    static QUEX_TYPE_CHARACTER   messaging_framework_data[] = 
       "hello 4711 bonjour 0815 world 7777 le 31451 monde le monde 00 welt 1234567890 hallo 1212 hello bye";
#else
    static ELEMENT_TYPE messaging_framework_data[] = 
       "Ελληνικά • Euskara • فارسی • Frysk • Galego • 한국어 • हिन्दी bye";
#endif
static size_t                messaging_framework_data_size()
{ 
    ELEMENT_TYPE* iterator = messaging_framework_data;
    for(; *iterator != 0; ++iterator);
    return (iterator - messaging_framework_data + 1) * sizeof(ELEMENT_TYPE);
}

size_t 
messaging_framework_receive(ELEMENT_TYPE** rx_buffer)
    /* Simulate the reception into a place that is defined by the low 
     * level driver. The low level driver reports the address of that place
     * and the size.                                                         */
{
    static ELEMENT_TYPE*  iterator = messaging_framework_data;
    const size_t          remainder_size =   messaging_framework_data_size() - 1 
                                           - (iterator - messaging_framework_data);
    size_t                size = (size_t)((float)(rand()) / (float)(RAND_MAX) * 5.0) + 1;

    if( size >= remainder_size ) size = remainder_size; 

    *rx_buffer = iterator; 
    iterator += size;

    if( size != 0 ) {
        __quex_assert(iterator < messaging_framework_data + messaging_framework_data_size());
    } else {
        __quex_assert(iterator == messaging_framework_data + messaging_framework_data_size());
    }

    return size;
}

size_t 
messaging_framework_receive_whole_characters(ELEMENT_TYPE** rx_buffer)
    /* Simulate the reception into a place that is defined by the low 
     * level driver. The low level driver reports the address of that place
     * and the size.                                                         */
{
    static ELEMENT_TYPE*  iterator = messaging_framework_data;
    const size_t          remainder_size =   messaging_framework_data_size() - 1 
                                           - (iterator - messaging_framework_data);
    size_t                size = (size_t)((float)(rand()) / (float)(RAND_MAX) * 5.0) + 1;

    if( size >= remainder_size ) size = remainder_size; 

    *rx_buffer = iterator; 
    iterator += size;

    /* We are dealing here with the UTF-8 type of message */
    __quex_assert(sizeof(ELEMENT_TYPE) == sizeof(uint8_t));

    /* If the two highest bits == '10' then it is a follow character in 
     * a utf8 encoded character. Thus, search for the first non '10' 
     * which indicates that we are pointing to a new letter.            */
    while( (*iterator & 0xC0) == 0x80 ) ++iterator;

    size = iterator - *rx_buffer;

    if( size != 0 ) {
        __quex_assert(iterator < messaging_framework_data + messaging_framework_data_size());
    } else {
        __quex_assert(iterator == messaging_framework_data + messaging_framework_data_size());
    }

    return size;
}

size_t 
messaging_framework_receive_syntax_chunk(ELEMENT_TYPE** rx_buffer)
    /* Simulate the reception into a place that is defined by the low 
     * level driver. The low level driver reports the address of that place
     * and the size.                                                         */
{
    size_t         index_list[] = {0, 10, 29, 58, 72, 89, 98};
    static size_t  cursor = 0;
    size_t         size   = (size_t)0;

    *rx_buffer = messaging_framework_data + index_list[cursor]; 

    /* Apply the messaging_framework_data + ... so that we compute in enties
     * of ELEMENT_TYPE* and not '1'. Size shall be the number of characters. */
    size =   (messaging_framework_data + index_list[cursor + 1]) 
           - (messaging_framework_data + index_list[cursor]);

    cursor += 1;

    return size;
}
void 
messaging_framework_release(ELEMENT_TYPE* buffer)
    /* A messaging framework that provide the address of the received content
     * (a rx buffer) usually requires to release the rx buffer buffer at some point
     * in time.                                                                       */
{
    /* nothing has to happen here, we are just happy. */
}

size_t 
messaging_framework_receive_into_buffer(ELEMENT_TYPE* BufferBegin, size_t BufferSize)
    /* Simulate a low lever driver that is able to fill a specified position in memory. */
{
    static ELEMENT_TYPE*  iterator = messaging_framework_data;
    size_t                size = (size_t)((float)(rand()) / (float)(RAND_MAX) * 5.0) + 1;

    assert(iterator < messaging_framework_data + messaging_framework_data_size());
    if( iterator + size >= messaging_framework_data + messaging_framework_data_size() - 1 ) 
        size = messaging_framework_data_size() - (iterator - messaging_framework_data) - 1; 
    if( size > BufferSize )    
        size = BufferSize;

    memcpy(BufferBegin, iterator, size);
    iterator += size;

    return size;
}

size_t 
messaging_framework_receive_into_buffer_syntax_chunk(ELEMENT_TYPE* BufferBegin, size_t BufferSize)
    /* Simulate a low lever driver that is able to fill a specified position in memory. */
{
    size_t         index_list[] = {0, 10, 29, 58, 72, 89, 98};
    static size_t  cursor = 0;

    /* Apply the messaging_framework_data + ... so that we compute in enties
     * of ELEMENT_TYPE* and not '1'. Size shall be the number of characters. */
    size_t size = (  (messaging_framework_data + index_list[cursor + 1]) 
                   - (messaging_framework_data + index_list[cursor])) * sizeof(ELEMENT_TYPE);

    if( size > BufferSize ) size = BufferSize; 

    /* If the target buffer cannot carry it, we drop it. */
    memcpy(BufferBegin, messaging_framework_data + index_list[cursor], size); 

    cursor += 1;

    return size;
}

ELEMENT_TYPE   MESSAGING_FRAMEWORK_BUFFER[MESSAGING_FRAMEWORK_BUFFER_SIZE];

size_t
messaging_framework_receive_to_internal_buffer()
    /* Simular a low level driver that iself has a hardware fixed position in memory 
     * which it fills on demand.                                                      */
{
    memcpy(MESSAGING_FRAMEWORK_BUFFER + 1, messaging_framework_data, 
           messaging_framework_data_size() * sizeof(ELEMENT_TYPE));
    return messaging_framework_data_size();
}

