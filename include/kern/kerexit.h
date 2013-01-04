
#ifndef __KEREXIT_H_
#define __KEREXIT_H_

#include <kern/kern_globals.h>
#include <common/request.h>

struct request* kerexit(struct task_descriptor* TD);
void kerent();
struct request* kerexittest(struct task_descriptor* TD);
void kerenttest();

void kerentirq();

#endif

