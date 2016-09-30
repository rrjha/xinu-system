/*  main.c  - main */

#include <xinu.h>

pid32 recver_id;
pid32 sender_id;

process sender(void)
{
	/* Simple sender: Send an int message */
	send(recver_id, (('H'<<8)|'i') & 0xFFFF);

	return OK;
}

process recver(void)
{
	/* Simple receiver: Receive an int message */
	umsg32 myMsg = receive();
	printf("Got message \"%c%c\" from sender\n",((myMsg>>8) & 0xFF), (myMsg & 0xFF));

	return OK;
}


process	main(void)
{
	recvclr();

	recver_id = create(recver, 4096, 50, "recver", 0);
	sender_id = create(sender, 4096, 50, "sender", 0);

	resume(recver_id);
	resume(sender_id);

	return OK;
}
