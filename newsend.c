/*  newsend.c  - new implementation of send */

#include <xinu.h>


/*------------------------------------------------------------------------
 *  send  -  Pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	sendMsg(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* Contents of message		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->nmsg.count == MAX_MSGS) {
		restore(mask);
		return SYSERR;
	}

	kprintf("snd: before eq: qin=%d, qout=%d, count=%d\n", prptr->nmsg.qin, prptr->nmsg.qout, prptr->nmsg.count);

	prptr->nmsg.msgq[prptr->nmsg.qin] = msg;		/* Queue message		*/
	prptr->nmsg.count++;
	prptr->nmsg.qin = (prptr->nmsg.qin+1)%MAX_MSGS;

	kprintf("snd: after eq: qin=%d, qout=%d, count=%d\n", prptr->nmsg.qin, prptr->nmsg.qout, prptr->nmsg.count);
	
	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		kprintf("Wake up process\n");
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return OK;
}

uint32	sendMsgs(
	pid32	pid,		/* ID of recipient process	*/
	umsg32	*msgs,	/* Array of messages	*/
	uint32	msg_count	/* number of messages	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int sent_count=0;

	mask = disable();
	if (isbadpid(pid) || (msgs == NULL) || (msg_count == 0)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];

	for (sent_count=0; (sent_count<msg_count) && (prptr->nmsg.count < MAX_MSGS);sent_count++) {
		prptr->nmsg.msgq[prptr->nmsg.qin] = msgs[sent_count];		/* Queue message		*/
		prptr->nmsg.count++;
		prptr->nmsg.qin = (prptr->nmsg.qin+1)%MAX_MSGS;
	}

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		kprintf("Wake up process\n");
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return sent_count;
}
