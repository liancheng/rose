#include <in/main.h>

void scan_init(size_t FileSize)
{
    yyin = global_fh; yyrestart(yyin);
}
