#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__

#define SHM_NAMES 0
#define SHM_NAMES_SIZE 1024

#define SHM_GOSSIP 1
#define SHM_GOSSIP_SIZE 4096

#define SHM_TOTAL_SIZE (SHM_NAMES_SIZE + SHM_GOSSIP_SIZE)

int* shmget(int zone) {
	switch(zone) {
		case SHM_NAMES:
			break;
	}
}


#endif

