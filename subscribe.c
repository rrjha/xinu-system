#include <xinu.h>


/* Topic table */
struct topic TOPIC_TABLE[MAX_TOPICS];

syscall  subscribe(topic16  topic,  void  (*handler)(topic16,  void*,  uint32))
{
	intmask	mask;			/* Saved interrupt mask		*/
	int32 i=0, slot=MAX_SUBSCRIBERS;
	uint8 grp = ((topic>>8)&0xFF), tpc = (topic&0xFF);

	mask = disable();

        if (handler == NULL) {
                restore(mask);
                return SYSERR;
        }

	/* Offset into topic table and check if the process is registered for any other group */
	/* Do we need to check entire table if process is registered in any other group for any topic? */
	for (i=0; i<MAX_SUBSCRIBERS; i++) {
		if(TOPIC_TABLE[tpc].subs[i].pid == -1)
			/* Free slot mark it */
			slot = i;
		if((TOPIC_TABLE[tpc].subs[i].pid == currpid) && (TOPIC_TABLE[tpc].subs[i].group != grp)){
			/* Process already registered with different group throw error */
			restore(mask);
			return SYSERR;
		}
		else if((TOPIC_TABLE[tpc].subs[i].pid == currpid) && (TOPIC_TABLE[tpc].subs[i].group == grp)){
			/* Process already registered with same group. Assume update of handler */
			slot = i;
			break;
		}
	}

	/* Slot = valid? then update */
	if (slot < MAX_SUBSCRIBERS) {
		TOPIC_TABLE[tpc].subs[slot].group = grp;
		TOPIC_TABLE[tpc].subs[slot].pid = currpid;
		TOPIC_TABLE[tpc].subs[slot].handler = handler;
	}
	else {
		/* No space left - return error */
		restore(mask);
		return SYSERR;
	}

	restore(mask);		/* Restore interrupts */
	return OK;
}


syscall  unsubscribe(topic16  topic)
{
	intmask mask;                   /* Saved interrupt mask         */
	int32 i=0;
	uint8 grp = ((topic>>8)&0xFF), tpc = (topic&0xFF);

	mask = disable();

	/* We don't care if the process didn't subscribe and is unsubscribing */
	for (i=0; i<MAX_SUBSCRIBERS; i++) {
		if((TOPIC_TABLE[tpc].subs[i].pid == currpid) && (TOPIC_TABLE[tpc].subs[i].group == grp)){
			/* Match */
			TOPIC_TABLE[tpc].subs[i].group = 0;
			TOPIC_TABLE[tpc].subs[i].pid = -1;
			TOPIC_TABLE[tpc].subs[i].handler = NULL;
			break;
		}
	}

	restore(mask);          /* Restore interrupts */
	return OK;
}

