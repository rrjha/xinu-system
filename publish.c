#include <xinu.h>
#include "subscriber.h"
#include "dbg.h"

extern struct topic TOPIC_TABLE[MAX_TOPICS];

process broker(topic16 topic, uint32  data)
{
	uint8 grp = (topic>>8)&0xFF;
	uint8 tpc = (topic&0xFF);
	uint32 i=0;

	if (!grp){
		/* Wildcard */
		for (i=0; i<MAX_SUBSCRIBERS; i++) {
			if((TOPIC_TABLE[tpc].subs[i].pid > 0) && (TOPIC_TABLE[tpc].subs[i].handler != NULL))
				TOPIC_TABLE[tpc].subs[i].handler(topic, data);
		}
	}
	else{
		for (i=0; i<MAX_SUBSCRIBERS; i++) {
			if((TOPIC_TABLE[tpc].subs[i].pid > 0) && (TOPIC_TABLE[tpc].subs[i].handler != NULL) && (TOPIC_TABLE[tpc].subs[i].group == grp))
				TOPIC_TABLE[tpc].subs[i].handler(topic, data);
		}
	}

	return OK;
}


syscall  publish(topic16  topic,  uint32  data)
{
	intmask	mask;			/* Saved interrupt mask		*/
	pid32 broker_pid = -1;
//	char broker_name[PNMLEN];
//	struct	procent *prptr;


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
