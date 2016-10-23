#include <xinu.h>
#include "subscriber.h"
#include "dbg.h"

extern sid32 mutex_console;

extern struct subscriber SUB_TABLE[MAX_GROUPS][MAX_TOPICS][MAX_SUBSCRIBERS];

process broker(topic16 topic, uint32  data)
{
	uint8 grp = (topic>>8)&0xFF;
	uint8 tpc = (topic&0xFF);
	uint32 i=0, grp_iter = grp, grp_max = grp+1;

	if(grp == 0){
		/* Wildcard - set the boundaries accordingly before looping */
		grp_iter = 1;
		grp_max = MAX_GROUPS;
	}

	for(grp_iter; grp_iter < grp_max; grp_iter++){
		for (i=0; i<MAX_SUBSCRIBERS; i++) {
			if((SUB_TABLE[grp_iter][tpc][i].spid > 0)  && (SUB_TABLE[grp_iter][tpc][i].handler != NULL)){
				/* Note that we don't use semaphore to lock here as thread-safety doesn't ensure reentrancy */
				/* And we can easily lock up the system based on handler. So the thread-safety is best left to handler */

				SUB_TABLE[grp_iter][tpc][i].handler(topic, data);
			}
		}
	}

	return OK;
}


syscall  publish(topic16  topic,  uint32  data)
{
	intmask	mask;			/* Saved interrupt mask		*/
	pid32 broker_pid = -1;
//	char broker_name[PNMLEN];
	struct	procent *prptr;


	mask = disable();
	/* Create a broker process for distribution */
	broker_pid = create(broker, 4096, 50, "broker", 2, topic, data);
	if(broker_pid < 0) {
		/* Ran out of memory */
		restore(mask);
		return SYSERR;
	}

	/* Rename the broker's name to be per process broker
	sprintf(broker_name, "%s%d", "broker", broker_pid);
	prptr = &proctab[broker_pid];
	strcpy(prptr->prname, broker_name);*/
	

	/* Resume the broker to continue with publishing */
	resume(broker_pid);

	restore(mask);
	return OK;
}
