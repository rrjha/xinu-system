/*  main.c  - main */

/* --------------------------------------------------------------------------------------------------------------------------------
  * 					Test cases
  *================================================================================================================================
  * Positive test cases
  * PT1) sendMsg can send more than one message sucessfully
  * PT2) sendMsgs can send multiple messages at one time
  * PT3) sendnMsg can send a message to more than one process at a time
  * PT4) receiveMsg can read message at the top of the Q of calling process
  * PT5) receiveMsgs returns with count items when items available in Q
  *
  * Negative test cases
  * NT1) When recipeint's Q is full no message from sendMsg/sendMsgs/sendnMsg should get delivered
  * NT2) When recipent's Q is partially occupied and the number of messages sent via sendMsgs is greater than empty slots then
  *         fill empty slots only and return number of messages sent successfully (< total count)
  * NT3) sendnMsg when invoked with multiple messages of which few are having buffer full then the message should be delivered
  *         to remaining processes and the count of such processes where message was delivered is returned.
  * NT4) receiveMsgs blocks if "count" messages are not available (and returns with data once all are available)
  *---------------------------------------------------------------------------------------------------------------------------------
  */

#include <xinu.h>
#define MCOUNT 6

pid32 recver_id1, recver_id2, recver_id3, sender_id;
sid32 sem_s = 0, sem_r1=0, sem_r2=0, sem_r3=0, mutex_console=0;

process sender(void)
{
	umsg32 msg = 123, nmsg[MCOUNT]={135, 246, 357, 468, 579, 680};
	int32 i=0, res = OK;
	pid32 pids[3] = {recver_id1, recver_id2, recver_id3};

	/* Start prints on fresh line after all regular DHCP and other warnings from Xinu */
	printf("\n");

	wait(sem_s);
	/* PT 1 */
	for (i=0; i<MCOUNT; i++) {
		res = sendMsg(recver_id1, msg);
		if (res == OK) {
			printf("Sucessfully sent message %d to pid: %d\n",msg, recver_id1);
			msg = msg + 111;
		}
		else {
			printf("Error\n");
			break;
		}
	}
	/* PT2 */
	res = sendMsgs(recver_id2, nmsg, MCOUNT);
	if(res != SYSERR) {
		printf("Successfully sent %d messages at one time : ", res);
		for (i=0; i<res;i++)
			printf("%d, ", nmsg[i]);
		printf("to pid %d\n", recver_id2);
	}
	/* PT3 */
	res = sendnMsg(3, pids, msg);
	if (res != SYSERR) {
		printf("Sent message %d to %d processes\n",msg, res);
		msg = msg + 111;
	}

	/* NT2 */
	res = sendMsgs(recver_id1, nmsg, MCOUNT);
	if(res != SYSERR) {
		printf("Successfully sent %d messages at one time : ", res);
		for (i=0; i<res;i++)
			printf("%d, ", nmsg[i]);
		printf("to pid %d\n", recver_id1);
	}

	/* NT1 - receiver1, NT3 */
	for (i=0; i<MCOUNT; i++) {
		res = sendnMsg(3, pids, msg);
		if (res != SYSERR) {
			printf("Sent message %d to %d processes\n",msg, res);
			msg = msg + 111;
		}
		else {
			printf("Error\n");
			break;
		}
	}
	signal(sem_r1);
	signal(sem_r2);
	signal (sem_r3);

	/* Send remaining message to unblock the waiting recver-1 */
	for (i=0; i<2; i++) {
		res = sendMsg(recver_id1, msg);
		if (res == OK) {
			wait(mutex_console);
			printf("Sucessfully sent message %d to pid: %d\n",msg, recver_id1);
			signal(mutex_console);
			msg = msg + 111;
		}
	}
	return OK;
}

process recver(sid32 sem_r)
{
	int32 i=0, res=SYSERR;
	umsg32 msg = 0, nmsg[MCOUNT]={0};
	wait(sem_r);
	/* PT4 - Receiver1*/
	for (i=0; i<MCOUNT; i++) {
		msg = receiveMsg();
		wait(mutex_console);
		printf("Received message %d in pid %d\n",msg, currpid);
		signal(mutex_console);
	}
	/* PT5/NT4 - Depending on when sender gets chance to send second set to Receiver 1 */
	res = receiveMsgs(nmsg, MCOUNT);
	if(res == OK) {
		wait(mutex_console);
		printf("Received %d messages at one time : ", MCOUNT);
		for(i=0; i<MCOUNT;i++)
			printf("%d, ", nmsg[i]);
		printf("in pid %d\n", currpid);
		signal(mutex_console);
	}

	return OK;
}


process	main(void)
{
	recvclr();
	sem_s = semcreate(3);
	sem_r1 = semcreate(0);
	sem_r2 = semcreate(0);
	sem_r3 = semcreate(0);
	mutex_console = semcreate(1);

	recver_id1 = create(recver, 4096, 50, "recver1", 1, sem_r1);
	recver_id2 = create(recver, 4096, 50, "recver2", 1, sem_r2);
	recver_id3 = create(recver, 4096, 50, "recver3", 1, sem_r3);
	sender_id = create(sender, 4096, 25, "sender", 0);

	resume(recver_id1);
	resume(recver_id2);
	resume(recver_id3);
	resume(sender_id);

	return OK;
}
