#ifndef __KERN_INIT__H__
#define __KERN_INIT__H__

void initialize_first_task(struct kern_globals* GLOBAL);
void initialize_events(struct kern_globals* GLOBAL);
void initialize_task_descriptors(struct kern_globals* GLOBAL);
void handle(struct kern_globals* GLOBAL, struct request* req);
void initialize_hardware();
void cleanup_hardware();
void initialize_interrupt_vec();

#endif
