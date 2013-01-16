#include <in/main.h>

char*  global_re2c_buffer_begin    = 0x0;
char*  global_re2c_buffer_end      = 0x0;
char*  global_re2c_buffer_iterator = 0x0;

void scan_init(size_t FileSize)
{
    global_re2c_buffer_begin    = (char*)malloc(sizeof(char)*(size_t)(FileSize * 2));
    global_re2c_buffer_iterator = global_re2c_buffer_begin;
    /* re2c does not provide the slightest buffer management, 
     * => load the whole bunch at once.                        */
    size_t Size = fread(global_re2c_buffer_begin, 1, (size_t)(FileSize * 2), global_fh);
    /* Set the terminating zero */
    *(global_re2c_buffer_begin + Size + 1) = '\0';
    global_re2c_buffer_end = global_re2c_buffer_begin + FileSize;
}
