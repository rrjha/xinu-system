#include <xinu.h>
#include "subscriber.h"


/* Subscriber table */
struct subscriber SUB_TABLE[MAX_GROUPS][MAX_TOPICS][MAX_SUBSCRIBERS]; /* Subscriber table of 255 groups (excl 0) * 256 topics * 8 Max process */

syscall  subscribe(topic16  topic,  void  (*handler)(topic16,  uint32))
{
	intmask	mask;			/* Saved interrupt mask		*/
	int i=0, j=0, k=0;
	uint8 grp = ((topic>>8)&0xFF), tpc = (topic&0xFF);
	static bool8 first_subscriber = TRUE;

	mask = disable();

	kprintf("Enter subscribe with topic=0x%04H and pid=%d\n", topic, currpid);

	if(first_subscriber)
	{
		/* Initialize the subscriber table */
//		memset(SUB_TABLE, 0, sizeof(SUB_TABLE));
		for (i=0; i<MAX_GROUPS; i++)
			for(j=0; j < MAX_TOPICS; j++)
				for(k=0; k<MAX_SUBSCRIBERS; k++) {
					SUB_TABLE[i][j][k].spid = -1;
					SUB_TABLE[i][j][k].handler = NULL;
				}
		first_subscriber = FALSE;
		kprintf("Initialized the topic table\n");
	}
		
	/* Topic values range from 0 to FFFF and topic is uint16 */
	/* So check only for handler */
	if (handler == NULL) {
		kprintf("Handler NULL for pid=%d\n", currpid);
		restore(mask);
		return SYSERR;
	}

	/* Check if this process is registered in any other group for this topic */
	for (i=0; i<MAX_GROUPS; i++) {
		for(j=0; j<MAX_SUBSCRIBERS;j++) {
			if((SUB_TABLE[i][tpc][j].spid == currpid) && (SUB_TABLE[i][tpc][j].handler != NULL)){
				kprintf("pid=%d exists in other group\n", currpid);
				restore(mask);
				return SYSERR;
			}
		}
	}

	/* Register this subscriber for supplied topic */
	for (i=0; i<MAX_SUBSCRIBERS; i++) {
		kprintf("Iter=%d, spid = %d, handler_address = %X\n", i, SUB_TABLE[grp][tpc][i].spid, SUB_TABLE[grp][tpc][i].handler);
		if(((SUB_TABLE[grp][tpc][i].spid) <= 0) && (SUB_TABLE[grp][tpc][i].handler == NULL)) {
			/* First empty slot found - insert the subscriber */
			SUB_TABLE[grp][tpc][i].spid = currpid;
			SUB_TABLE[grp][tpc][i].handler= handler;
			kprintf("Inserted in group=%02u, topic = %02u at pos %d, spid = %d, handler_address = 0x%X\n", grp, tpc, i, SUB_TABLE[grp][tpc][i].spid, SUB_TABLE[grp][tpc][i].handler);
			break;
		}
	}

	/* No space left to subscribe */
	if(i == MAX_SUBSCRIBERS) {
		kprintf("no space\n");
		restore(mask);
		return SYSERR;
	}

	kprintf("Exit subscribe for pid=%d\n", currpid);
	restore(mask);		/* Restore interrupts */
	return OK;
}

syscall  unsubscribe(topic16  topic)
{
	intmask	mask;			/* Saved interrupt mask		*/
	int i=0, j=0;
	uint8 grp = ((topic>>8)&0xFF), tpc = (topic&0xFF);

	mask = disable();

	/* We don't care if the process didn't subscribe and is unsubscribing */
	for (i=0; i<MAX_SUBSCRIBERS; i++) {
		if(SUB_TABLE[grp][tpc][i].spid == currpid) {
			/* First empty slot found - insert the subscriber */
			SUB_TABLE[grp][tpc][i].spid = -1;
			SUB_TABLE[grp][tpc][i].handler= NULL;
		}
	}

	restore(mask);		/* Restore interrupts */
	return OK;
}
