/* receive.c - receive */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receive  -  Wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receive(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	mask = disable();
	prptr = &proctab[currpid];
	if (prptr->prhasmsg == FALSE) {
		prptr->prstate = PR_RECV;
		resched();		/* Block until message arrives	*/
	}
	msg = prptr->prmsg;		/* Retrieve message		*/
	prptr->prhasmsg = FALSE;	/* Reset message flag		*/
	restore(mask);
	return msg;
}


/*------------------------------------------------------------------------------
 *  receive  -  Reads a message from the message queue if available else waits
 *              for a message and then read. Returns the message to the caller
 *  Parameters:
 *  None
 *  Return
 *  A message (umsg32) - First message in the queue
 *-------------------------------------------------------------------------------
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


/*------------------------------------------------------------------------------------
 *  receiveMsgs - Receieve a group (msg_count) of messages (msgs)
 *  Parameters:
 *  msgs = Out parameter that will contain the base address of array holding messages
 *  msg_count = Number of messages
 *  Return
 *  SYSERR if out parameter msgs is NULL or count of messages is 0
 *  OK otherwise
 *----------------------------------------------------------------------------------
 */

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
