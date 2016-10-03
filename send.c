/* send.c - send */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  send  -  Pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	send(
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
	if (prptr->prhasmsg) {
		restore(mask);
		return SYSERR;
	}
	prptr->prmsg = msg;		/* Deliver message		*/
	prptr->prhasmsg = TRUE;		/* Indicate message is waiting	*/

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


#define MAX_RECVERS 3 /* Maximum number of receivers is limited to 3 currently */

/*------------------------------------------------------------------------------
 *  sendMsg  -  Queue a message for a process and ready recipient if waiting
 *  Parameters:
 *  pid = process id of recepient
 *  msg = message to send/enqueue
 *  Return
 *  OK - When the message is enqueued
 *  SYSERR - When the pid of the process is invalid or recipient's buffer is full
 *-------------------------------------------------------------------------------
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

/*----------------------------------------------------------------------------------
 *  sendMsgs  -  Queue multiple message for a process and ready recipient if waiting
 *  Parameters:
 *  pid = process id of recepient
 *  msgs = base address of array holding messages to send/enqueue
 *  msg_count = Number of messages
 *  Return
 *  number (uint32) - Number of messages sucessfully queued.
 *  SYSERR - For any of the conditions listed below:
 *  a) When the pid of the process is invalid
 *  b) List of msgs is NULL or count of messages is 0
 *  c) Recipient's buffer is full
 *----------------------------------------------------------------------------------
 */

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

        if (prptr->nmsg.count == MAX_MSGS) {
                restore(mask);
                return SYSERR;
        }

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

/*-----------------------------------------------------------------------------------------
 *  sendnMsg  -  Queue a message for multiple processes and ready recipient(s) if waiting
 *  Parameters:
 *  pid_count = Number of processes to send this message to.
 *  pids = process ids of recepients
 *  msg = message to send
 *  Return
 *  number (uint32) - Number of recipient processes where message was sucessfully queued
 *  SYSERR - For any of the conditions listed below:
 *  a) When the pid array is NULL
 *  b) When either pid count is 0 or more than MAX allowed
 *  c) When the pid of any process is invalid
 *
 *  NOTE:
 *  1) This call DOESN'T return SYSERR when the receive buffer for one or more processes
 *     is overflowing. This is so as to allow other processes to recive messgaes.
 *     As a fallout of this a successful return could have values from 0 to pid_count.
 *     The reasoning behind this is a broadcasting process may not need to know the
 *     status of the message queue of all the receipient processes.
 *  2) The call returns SYSERR if any one of the process is invalid. This is so that
 *     there is no malicious intent and any process being invalid indicates that sender
 *     is not aware of system state (e.g. a stale query of all pids available). Hence any
 *     service of such call must be denied.
 *  3) Current implementation tries to deliver msg to all recipents. Any buffer-overflow
 *     error encountered in the process will be ignored and sender will try to send to
 *     subsequent recipient in the list, if any left. And at the end return count of
 *     successful sends (no overflow). However, the sender has no way of knowing which
 *     recipeints got the message in this case. Based on requirement guideline
 *     and/or a different signature of API we could define a logic where we return
 *     failure for first buffer-overflow encountered or a list of failing PIDs, if
 *     a retry mechanism at sender's side is required.
 *-----------------------------------------------------------------------------------------
 */

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
