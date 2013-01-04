/* system_nameserver.c - code for nameserver */
#include <userspace.h>

struct name_entry * find_name_entry(struct name_entry * name_entries, int num_entries, char * name);

//Individual list entries in the nemeserver
struct name_entry {
	char name[CONFIG_NAMESRV_NAME_LENGTH];
	int tid;
	struct name_entry * next;
};

void task_system_nameserver() {
	struct name_entry name_entries[CONFIG_NAMESRV_MAX_NAME_ENTRIES];
	struct name_entry * cur = NULL;
	int tid = -1, status = -1, i = 0;
	int next_free_entry = 0;
	
	//Initialize array of name entries
	for(i = 0; i < CONFIG_NAMESRV_MAX_NAME_ENTRIES; i++) {
		name_entries[i].next = NULL;
	}
	
	struct nameserver_message msg;
	struct number_message rep;
	while(1) {
		status = Receive(&tid, (char*)&msg, sizeof(msg));
		assert(status == sizeof(msg),"status == sizeof(msg)")
		switch(msg.message_type) {
			case REGISTERAS_MESSAGE:
				//Register a new task to the nameserver
				//bwprintf(COM2,"Nameserver: RegisterAs - Registering task %d as '%s'\r\n", tid, msg.name);	
				rep.message_type = NUMBER_MESSAGE;
				rep.num = 0;
				cur = find_name_entry(name_entries, next_free_entry, msg.name);
				if(cur == NULL) {
					//We don't already know about this task - good, we can add it
					name_entries[next_free_entry].tid = tid;
					safestrcpy(name_entries[next_free_entry].name,msg.name,CONFIG_NAMESRV_NAME_LENGTH);
					//Add it to the list of the previous entry
					if(next_free_entry > 0) {
						name_entries[next_free_entry - 1].next = name_entries + next_free_entry;
					}
					name_entries[next_free_entry].next = NULL;
					next_free_entry++;					
				} else {
					//Duplicate name!
					rep.num = NAME_DUPLICATE;
				}
				//bwprintf(COM2,"Nameserver: RegisterAs - Registered task %d as '%s'\r\n", tid, msg.name);
				Reply(tid, (char*)&rep, sizeof(rep));
				break;
			case WHOIS_MESSAGE:
				//Lookup a task number in the nameserver
				rep.message_type = NUMBER_MESSAGE;
				rep.num = NAME_NOT_FOUND;
				
				cur = find_name_entry(name_entries, next_free_entry, msg.name);				
				if(cur != NULL) {
					rep.num = cur->tid;
					//bwprintf(COM2,"Nameserver: WhoIs - Found task '%s' has tid %d\r\n", msg.name, cur->tid);
				}
				
				Reply(tid, (char*)&rep, sizeof(rep));				
				break;
			default:
				//We received an invalid operation!
				bwprintf(COM2,"Nameserver: received invalid message type: %d from task %d\r\n", msg.message_type, tid);
				rep.message_type = INVALID_OPERATION_MESSAGE;
				rep.num = -1;
				Reply(tid, (char*)&rep, sizeof(rep));
				assert(status == 0,"status == 0")
				break;
		}
	}
}

//Find a name entry in the list of nameservers
struct name_entry * find_name_entry(struct name_entry * name_entries, int num_entries, char * name) {
	struct name_entry * cur = NULL;	
	if(num_entries > 0) {
		cur = name_entries;
		//Follow linked list of pointers
		while(cur != NULL) {
			if(strcmp(name, cur->name) == 0) {
				return cur;
			}
			cur = cur->next;
		}
	}
	return NULL;
}
