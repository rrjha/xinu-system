/*  main.c  - main */

#include <xinu.h>

#define sync_print(x) \
		wait(mutex_console);\
		printf x;\
		signal(mutex_console);


pid32 publisher_id, subscriber1_id, subscriber2_id;

sid32 mutex_console=0;
extern struct topic TOPIC_TABLE[MAX_TOPICS];

extern struct publishQEntry *Qhead, *Qtail;

struct publishQEntry* get_next_pending_publish()
{
	struct publishQEntry *temp = NULL;
	intmask	mask;			/* Saved interrupt mask		*/

	if(Qhead != NULL){
		/* Read and store the head */
		temp = Qhead;
		/* Update the head for next read */
		if(Qhead != Qtail){
			Qhead = Qhead->next;
		}
		else {
			/* We need to run this part in atomic fashion to allow correct update */
			mask = disable();
			if(Qhead == Qtail){
				/* This is last item so update that Q is empty after returning this item */
				Qhead = NULL;
				Qtail = NULL;
			}
			restore(mask);
		}
		temp->next = NULL;
	}

	return temp; /* Caller to free mem once done */
}

process broker()
{
	struct publishQEntry *pubItem = NULL;
	uint8 grp;
	uint8 tpc;
	uint32 i=0;

	while(1){
		pubItem = get_next_pending_publish();
		if(pubItem != NULL){
			grp = (pubItem->topic>>8)&0xFF;
			tpc = (pubItem->topic & 0xFF);
			if (!grp){
				/* Wildcard */
				for (i=0; i<MAX_SUBSCRIBERS; i++) {
					if((TOPIC_TABLE[tpc].subs[i].pid > 0) && (TOPIC_TABLE[tpc].subs[i].handler != NULL))
						TOPIC_TABLE[tpc].subs[i].handler(pubItem->topic, pubItem->data);
				}
			}
			else{
				for (i=0; i<MAX_SUBSCRIBERS; i++) {
					if((TOPIC_TABLE[tpc].subs[i].pid > 0) && (TOPIC_TABLE[tpc].subs[i].handler != NULL) && (TOPIC_TABLE[tpc].subs[i].group == grp))
						TOPIC_TABLE[tpc].subs[i].handler(pubItem->topic, pubItem->data);
				}
			}

			freemem((char*)pubItem, sizeof(struct publishQEntry));
		}
		else
			/* Sleep for a while and poll again */
			sleepms(100);
	}

	return OK;
}


void cb_subscriber1(topic16 topic,  uint32 data)
{
	sync_print(("Function cb_subscriber1() is called with topic16=0x%04X and data=0x%X \n", topic, data));
}


void cb_subscriber2(topic16 topic,  uint32 data)
{
	sync_print(("Function cb_subscriber2() is called with topic16 0x%04X and data 0x%X \n", topic, data));
}

/* Publisher */
process publisher(void)
{
	topic16 tpc = 0x013F;
	uint32 data = 0xFF;
	int32 res = publish(tpc, data);
	if(res == OK) {
		sync_print(("Process pid-%d publishes  data  0x%X  to  topic16 0x%04X\n", publisher_id, data, tpc));
	}

	tpc = 0x023F;
	data = 0xDD;
	res = publish(tpc, data);
	if(res == OK) {
		sync_print(("Process pid-%d publishes  data  0x%X  to  topic16 0x%04X\n", publisher_id, data, tpc));
	}

	sleep(5);

	tpc = 0x003F;
	data = 0xAA;
	res = publish(tpc, data);
	if(res == OK) {
		sync_print(("Process pid-%d publishes  data  0x%X  to  topic16 0x%04X\n", publisher_id, data, tpc));
	}

	return OK;
}

/* Subscriber2 */
process subscriber1(void)
{
	int32 res=SYSERR;
	topic16 tpc = 0x013F;
	res = subscribe(tpc, cb_subscriber1);
	if(res == OK) {
		sync_print(("Process pid-%d subscribed with a topic16 value of 0x%04X and handler cb_subscriber1()\n", subscriber1_id, tpc));
	}
	sleep(2);
//	unsubscribe(tpc);
	return OK;
}


/* Subscriber2 */
process subscriber2(void)
{
	int32 res=SYSERR;
	topic16 tpc = 0x023F;
	res = subscribe(tpc, cb_subscriber2);
	if(res == OK) {
		sync_print(("Process pid-%d subscribed with a topic16 value of 0x%04X and handler cb_subscriber2()\n", subscriber2_id, tpc));
	}
	sleep(10);
//	unsubscribe(tpc);
	return OK;
}

process	main(void)
{
	pid32 broker_pid;
	int32 i, j;

	recvclr();

	Qhead = NULL;
	Qtail = NULL;

	mutex_console = semcreate(1);

	/* Initialize the topic table */
	for (i=0; i<MAX_TOPICS; i++) {
	       for(j=0; j < MAX_SUBSCRIBERS; j++){
			TOPIC_TABLE[i].subs[j].group = 0;
			TOPIC_TABLE[i].subs[j].pid = -1;
			TOPIC_TABLE[i].subs[j].handler = NULL;
	       }
	}

	/* Create a broker process for distribution */
	broker_pid = create(broker, 4096, 50, "broker", 0);

	publisher_id = create(publisher, 4096, 50, "publisher", 0);
	subscriber1_id = create(subscriber1, 4096, 50, "subscriber1", 0);
	subscriber2_id = create(subscriber2, 4096, 50, "subscriber2", 0);
	printf("publisher_id = %d, sub1_id = %d, sub2_id=%d\n",publisher_id, subscriber1_id, subscriber2_id);

	/* Resume subscriber-1 first */
	resume(subscriber1_id);

	resume(subscriber2_id);

	resume(broker_pid);
	resume(publisher_id);

	return OK;
}
