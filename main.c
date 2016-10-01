/*  main.c  - main */

#include <xinu.h>

pid32 recver_id;
pid32 sender_id;
sid32 sem_s = 0, sem_r=0;

process sender(void)
{
	/* Simple sender: Send an int message */
//	sendMsg(recver_id, (('H'<<8)|'i') & 0xFFFF);
	umsg32 msg = 123;
	int32 i=0, res = OK;
	wait(sem_s);
	for (i=0; i<3; i++) {
		res = sendMsg(recver_id, msg);
		if (res == OK) {
			printf("Sent message %d\n",msg);
			msg = msg + 111;
		}
		else {
			printf("Error\n");
			break;
		}
	}
	signal(sem_r);
	return OK;
}

process recver(void)
{
	/* Simple receiver: Receive an int message */
	int32 i=0;
	umsg32 msg = 0;
//	printf("Got message \"%c%c\" from sender\n",((msg>>8) & 0xFF), (msg & 0xFF));
	wait(sem_r);
	for (i=0; i<3; i++) {
		msg = receiveMsg();
		printf("Received message %d\n",msg);
	}
	signal(sem_s);

	return OK;
}


process	main(void)
{
	recvclr();
	sem_s = semcreate(3);
	sem_r = semcreate(0);
	recver_id = create(recver, 4096, 50, "recver", 0);
	sender_id = create(sender, 4096, 50, "sender", 0);

	resume(recver_id);
	resume(sender_id);

	return OK;
}
