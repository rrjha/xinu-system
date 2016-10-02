/*  newsend.c  - new implementation of send */

#include <xinu.h>
#define MAX_RECVERS 3

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


	prptr->nmsg.msgq[prptr->nmsg.qin] = msg;		/* Queue message */
	prptr->nmsg.count++;
	prptr->nmsg.qin = (prptr->nmsg.qin+1)%MAX_MSGS;


	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
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
	umsg32	*msgs,		/* Array of messages		*/
	uint32	msg_count	/* number of messages		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	uint32 sent_count=0;

	mask = disable();
	if (isbadpid(pid) || (msgs == NULL) || (msg_count == 0)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];

	for (sent_count=0; (sent_count<msg_count) && (prptr->nmsg.count < MAX_MSGS);sent_count++) {
		prptr->nmsg.msgq[prptr->nmsg.qin] = msgs[sent_count];		/* Queue message */
		prptr->nmsg.count++;
		prptr->nmsg.qin = (prptr->nmsg.qin+1)%MAX_MSGS;
	}

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return sent_count;
}

uint32	sendnMsg(
	uint32	pid_count,	/* Number of recipients	*/
	pid32*	pids,		/* List of pids		*/
	umsg32	msg		/* Msg to send		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int i=0, sent_pid=0;

	mask = disable();
	if((pids == NULL) || (pid_count == 0) || (pid_count > MAX_RECVERS)) {
		restore(mask);
		return SYSERR;
	}
	for (i=0; i<pid_count; i++)
		if (isbadpid(pids[i])) {
			restore(mask);
			return SYSERR;
		}

	resched_cntl(DEFER_START);
	for (i=0; i<pid_count; i++) {
		prptr = &proctab[(pids[i])];
		if (prptr->nmsg.count != MAX_MSGS) {
			prptr->nmsg.msgq[prptr->nmsg.qin] = msg;		/* Queue message */
			prptr->nmsg.count++;
			prptr->nmsg.qin = (prptr->nmsg.qin+1)%MAX_MSGS;
			sent_pid++;
		}

		/* If recipient waiting or in timed-wait make it ready */
		if (prptr->prstate == PR_RECV) {
			ready(pids[i]);
		} else if (prptr->prstate == PR_RECTIM) {
			unsleep(pids[i]);
			ready(pids[i]);
		}
	}
	resched_cntl(DEFER_STOP);

	restore(mask);		/* Restore interrupts */
	return sent_pid;
}
