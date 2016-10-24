#include <xinu.h>

extern struct topic TOPIC_TABLE[MAX_TOPICS];

/* Dyanmic Q for publish */
struct publishQEntry *Qhead, *Qtail;

int32 enqueue_publish_data(topic16  topic,  uint32  data)
{
       struct publishQEntry *newEntry = NULL;

       newEntry = (struct publishQEntry *) getmem(sizeof(struct publishQEntry));
       if(*((char*)newEntry) == SYSERR){
               /* We are out of memory - return error */
               return SYSERR;
       }
       newEntry->data = data;
       newEntry->topic = topic;
	newEntry->next = NULL;

       if(Qtail == NULL){
               /* Empty Q - This is the first element */
               Qtail = newEntry;
               Qhead = Qtail;
       }
       else {
               Qtail->next = newEntry;
               Qtail = newEntry;
       }

       return OK;
}


syscall  publish(topic16  topic,  uint32  data)
{
	intmask	mask;			/* Saved interrupt mask		*/
	int32 res = SYSERR;

	mask = disable();

	/* Enqueue the data */
	res = enqueue_publish_data(topic, data);
	if(res != OK){
		restore(mask);
		return SYSERR;
	}

	restore(mask);
	return OK;
}
