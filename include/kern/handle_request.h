#ifndef __KERN_HANDLE_REQUEST__
#define __KERN_HANDLE_REQUEST__

#include <kern/kern_globals.h>
#include <common/request.h>

void handle_request(struct kern_globals* GLOBAL, struct request* req);

#endif
