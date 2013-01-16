#include <in/main.h>

quex::quex_scan*  global_qlex; 

void scan_init(size_t FileSize)
{
    global_qlex = new quex::quex_scan(global_fh);
}
