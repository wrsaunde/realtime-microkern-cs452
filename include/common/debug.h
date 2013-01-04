#ifndef __DEBUG__H__
#define __DEBUG__H__


#ifndef NDEBUG

#include <lib/bwio.h>
#define assert(cond, msg) if(!(cond)) { bwprintf(COM2, "\x1B[2JAssert failed (%s:%d): %s \r\n\r\n", __FILE__, __LINE__, (msg)); Quit(); }
#define kernel_assert(cond, msg) if(!(cond)) { bwprintf(COM2, "\x1B[2JAssert failed (%s:%d): %s \r\n\r\n", __FILE__, __LINE__, (msg)); bwgetc(COM2); }

#endif

#endif
