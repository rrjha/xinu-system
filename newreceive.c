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
	kprintf("rcv: before dq: qin=%d, qout=%d, count=%d\n", prptr->nmsg.qin, prptr->nmsg.qout, prptr->nmsg.count);
	msg = prptr->nmsg.msgq[prptr->nmsg.qout];	/* Retrieve message		*/
	prptr->nmsg.qout = (prptr->nmsg.qout+1)%MAX_MSGS;
	prptr->nmsg.count--;
	kprintf("rcv: after dq: qin=%d, qout=%d, count=%d\n", prptr->nmsg.qin, prptr->nmsg.qout, prptr->nmsg.count);

	restore(mask);
	return msg;
}

