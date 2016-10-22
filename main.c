/*  main.c  - main */

#include <xinu.h>
#include "dbg.h"

pid32 publisher_id;
pid32 subscriber1_id;
pid32 subscriber2_id;

bool8 cb1_rcved = FALSE, cb2_rcved = FALSE;


sid32 mutex_console=0;

void cb_subscriber1(topic16 topic,  uint32 data)
{
	sync_print(("Function cb_subscriber1() is called with topic16=0x%04X and data=0x%X \n", topic, data));
	cb1_rcved = TRUE;
}


void cb_subscriber2(topic16 topic,  uint32 data)
{
	sync_print(("Function cb_subscriber2() is called with topic16 0x%04X and data 0x%X \n", topic, data));
	cb2_rcved = TRUE;
}

/* Publisher */
process publisher(void)
{
	topic16 tpc = 0x013F;
	uint32 data = 0xFF;
	int res = publish(tpc, data);
	if(res == OK) {
		sync_print(("Process pid-%d publishes  data  %d  to  topic16 0x%04X\n", publisher_id, data, tpc))
	}

	tpc = 0x023F;
	data = 0x55;
	res = publish(tpc, data);
	if(res == OK) {
		sync_print(("Process pid-%d publishes  data  %d  to  topic16 0x%04X\n", publisher_id, data, tpc))
	}

	return OK;
}

/* Subscriber2 */
process subscriber1(void)
{
	static int cnt=1;
	int res=SYSERR;
	topic16 tpc = 0x013F;
	res = subscribe(tpc, cb_subscriber1);
	if(res == OK) {
		sync_print(("Process pid-%d subscribed with a topic16 value of 0x%04X and handler cb_subscriber1()\n", subscriber1_id, tpc))
	}
	while(1) {
		if(cb1_rcved == FALSE)
			sleep(1);
		else
			break;
	}
	unsubscribe(tpc);
	return OK;
}


/* Subscriber2 */
process subscriber2(void)
{
	static int cnt=1;
	int res=SYSERR;
	topic16 tpc = 0x023F;
	res = subscribe(tpc, cb_subscriber2);
	if(res == OK) {
		sync_print(("Process pid-%d subscribed with a topic16 value of 0x%04X and handler cb_subscriber2()\n", subscriber2_id, tpc))
	}
	while(1) {
		if(cb2_rcved == FALSE)
			sleep(1);
		else
			break;
	}
	unsubscribe(tpc);
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
	printf("publisher_id = %d, sub1_id = %d, sub2_id=%d\n",publisher_id, subscriber1_id, subscriber2_id);

	/* Resume subscriber-1 first */
	resume(subscriber1_id);

	resume(subscriber2_id);

	resume(publisher_id);
	
	return OK;
}
