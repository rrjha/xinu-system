#include <xinu.h>
#include "subscriber.h"


/* Subscriber table */
struct subscriber SUB_TABLE[MAX_GROUPS][MAX_TOPICS][MAX_SUBSCRIBERS] = {0}; /* Subscriber table of 255 groups (excl 0) * 256 topics * 8 Max process */

syscall  subscribe(topic16  topic,  void  (*handler)(topic16,  uint32))
{
	intmask	mask;			/* Saved interrupt mask		*/
	int i=0, j=0;

	mask = disable();
	/* Topic values range from 0 to FFFF and topic is uint16 */
	/* So check only for handler */
	if (handler == NULL) {
		restore(mask);
		return SYSERR;
	}

	/* Check if this process is registered in any other group for this topic */
	for (i=0; i<MAX_GROUPS; i++) {
		for(j=0; j<MAX_SUBSCRIBERS;j++) {
			if(SUB_TABLE[i][topic&0xFF][j].spid == currpid) {
				restore(mask);
				return SYSERR;
			}
		}
	}

	/* Register this subscriber for supplied topic */
	for (i=0; i<MAX_SUBSCRIBERS; i++) {
		if(SUB_TABLE[(topic>>8)&0xFF][topic&0xFF][i].handler == NULL) {
			/* First empty slot found - insert the subscriber */
			SUB_TABLE[(topic>>8)&0xFF][topic&0xFF][i].spid = currpid;
			SUB_TABLE[(topic>>8)&0xFF][topic&0xFF][i].handler= handler;
		}
	}

	/* No space left to subscribe */
	if(i == MAX_SUBSCRIBERS) {
		restore(mask);
		return SYSERR;
	}

	restore(mask);		/* Restore interrupts */
	return OK;
}

syscall  unsubscribe(topic16  topic)
{
	intmask	mask;			/* Saved interrupt mask		*/
	int i=0, j=0;

	mask = disable();

	/* We don't care if the process didn't subscribe and is unsubscribing */
	for (i=0; i<MAX_SUBSCRIBERS; i++) {
		if(SUB_TABLE[(topic>>8)&0xFF][topic&0xFF][i].spid == currpid) {
			/* First empty slot found - insert the subscriber */
			SUB_TABLE[(topic>>8)&0xFF][topic&0xFF][i].spid = -1;
			SUB_TABLE[(topic>>8)&0xFF][topic&0xFF][i].handler= NULL;
		}
	}

	restore(mask);		/* Restore interrupts */
	return OK;
}
