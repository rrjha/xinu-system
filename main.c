/*  main.c  - main */

#include <xinu.h>

pid32 publisher_id;
pid32 subscriber1_id;
pid32 subscriber2_id;

bool8 cb1_rcved = FALSE, cb2_rcved = FALSE;


sid32 mutex_console=0;

void cb_subscriber1(topic16 topic,  uint32 data)
{
	wait(mutex_console);
	printf("Function cb_subscriber1() is called with topic16 %hX and data %X \n");
	cb1_rcved = TRUE;
	signal(mutex_console);
}


void cb_subscriber2(topic16 topic,  uint32 data)
{
	wait(mutex_console);
	printf("Function cb_subscriber2() is called with topic16 %hX and data %X \n");
	cb2_rcved = TRUE;
	signal(mutex_console);
}

/* Publisher */
process publisher(void)
{
	/* */
	publish(0x013F, 44);
	publish(0x023F, 55);
	return OK;
}

/* Subscriber2 */
process subscriber1(void)
{
	subscribe(0x013F, cb_subscriber1);
	printf("Subscriber1 done\n");
	while(!cb1_rcved); /* Wait here for cb */
	unsubscribe(0x013F);
	return OK;
}


/* Subscriber2 */
process subscriber2(void)
{
	subscribe(0x023F, cb_subscriber1);
	printf("Subscriber2 done\n");
	while(!cb2_rcved); /* Wait here for cb */
	unsubscribe(0x023F);
	return OK;
}

process	main(void)
{
	int32 ch=0;
	recvclr();

	mutex_console = semcreate(1);

	publisher_id = create(publisher, 4096, 50, "publisher", 0);
	subscriber1_id = create(subscriber1, 4096, 50, "subscriber1", 0);
	subscriber2_id = create(subscriber2, 4096, 50, "subscriber2", 0);

	/* Resume subscriber-1 first */
	resume(subscriber1_id);

	ch = getc(stdin);

	if((ch == 'y') || (ch == 'Y'))
		resume(subscriber2_id);

	ch = getc(stdin);

	if((ch == 'y') || (ch == 'Y'))
		resume(publisher_id);
	
	return OK;
}
