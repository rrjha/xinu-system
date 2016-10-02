/* newreceive.c - new receive syscall */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receive  -  Wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receiveMsg(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	mask = disable();
	prptr = &proctab[currpid];

	if (prptr->nmsg.count == 0) {
		prptr->prstate = PR_RECV;
		resched();		/* Block until message arrives	*/
	}
	msg = prptr->nmsg.msgq[prptr->nmsg.qout];	/* Retrieve message		*/
	prptr->nmsg.qout = (prptr->nmsg.qout+1)%MAX_MSGS;
	prptr->nmsg.count--;

	restore(mask);
	return msg;
}

syscall	receiveMsgs(
	umsg32 *msgs,	/* Out buffer to store messages - Caller needs to allocate sufficient buffer */
	uint32 msg_count
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/
	uint32 i = 0;
	mask = disable();
	prptr = &proctab[currpid];

	if ((msgs == NULL) || (msg_count == 0)) {
		restore(mask);
		return SYSERR;
	}

	if (prptr->nmsg.count < msg_count) {
		prptr->prstate = PR_RECV;
		resched();		/* Block until message arrives	*/
	}

	for (i=0; i<msg_count;i++) {
		msgs[i] = prptr->nmsg.msgq[prptr->nmsg.qout];	/* Retrieve message		*/
		prptr->nmsg.qout = (prptr->nmsg.qout+1)%MAX_MSGS;
		prptr->nmsg.count--;
	}

	restore(mask);
	return OK;
}
