/*  main.c  - main */

#include <xinu.h>
#define MCOUNT 3

pid32 recver_id1, recver_id2, sender_id;
sid32 sem_s = 0, sem_r1=0, sem_r2=0, mutex_console=0;

process sender(void)
{
	/* Simple sender: Send an int message */
//	sendMsg(recver_id, (('H'<<8)|'i') & 0xFFFF);
	umsg32 msg = 123, nmsg[MCOUNT]={135, 246, 579};
	int32 i=0, res = OK;
	pid32 pids[2] = {recver_id1, recver_id2};
	wait(sem_s);
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
	res = sendMsgs(recver_id1, nmsg, MCOUNT);
	if(res != SYSERR) {
		printf("Successfully sent %d messages at one time : ", res);
		for (i=0; i<res;i++)
			printf("%d, ", nmsg[i]);
		printf("to pid %d\n", recver_id1);
	}

	for (i=0; i<MCOUNT; i++) {
		res = sendnMsg(2, pids, msg);
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

	res = sendMsg(recver_id1, msg);
	if (res == OK) {
		wait(mutex_console);
		printf("Sucessfully sent message %d to pid: %d\n",msg, recver_id1);
		signal(mutex_console);
	}
	return OK;
}

process recver(sid32 sem_r)
{
	/* Simple receiver: Receive an int message */
	int32 i=0, res=SYSERR;
	umsg32 msg = 0, nmsg[MCOUNT]={0};
//	printf("Got message \"%c%c\" from sender\n",((msg>>8) & 0xFF), (msg & 0xFF));
	wait(sem_r);
	for (i=0; i<MCOUNT; i++) {
		msg = receiveMsg();
		wait(mutex_console);
		printf("Received message %d in pid %d\n",msg, currpid);
		signal(mutex_console);
	}
	res = receiveMsgs(nmsg, MCOUNT);
	if(res == OK) {
		wait(mutex_console);
		printf("Received %d messages at one time : ", MCOUNT);
		for(i=0; i<MCOUNT;i++)
			printf("%d, ", nmsg[i]);
		printf("in pid %d\n", currpid);
		signal(mutex_console);
	}

	for (i=0; i<MCOUNT; i++) {
		msg = receiveMsg();
		wait(mutex_console);
		printf("Received message %d in pid %d\n",msg, currpid);
		signal(mutex_console);
	}
	signal(sem_s);

	return OK;
}


process	main(void)
{
	recvclr();
	sem_s = semcreate(3);
	sem_r1 = semcreate(0);
	sem_r2 = semcreate(0);
	mutex_console = semcreate(1);

	recver_id1 = create(recver, 4096, 50, "recver1", 1, sem_r1);
	recver_id2 = create(recver, 4096, 50, "recver2", 1, sem_r2);
	sender_id = create(sender, 4096, 50, "sender", 0);

	resume(recver_id1);
	resume(recver_id2);
	resume(sender_id);

	return OK;
}
