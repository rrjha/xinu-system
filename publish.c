#include <xinu.h>

/* Dyanmic Q for publish */
struct publishQEntry *Qhead, *Qtail;

int32 enqueue_publish_data(topic16 topic, void* data, uint32 size)
{
       struct publishQEntry *newEntry = NULL;

       newEntry = (struct publishQEntry *) getmem(sizeof(struct publishQEntry));
	newEntry->data = (void*) getmem(size);
       if((*((char*)newEntry) == SYSERR) ||(*((char*)newEntry->data) == SYSERR)) {
               /* We are out of memory - return error */
               return SYSERR;
       }
       newEntry->topic = topic;
	newEntry->next = NULL;
	newEntry->size = size;
       memcpy(newEntry->data, data, size);

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


syscall  publish(topic16 topic, void* data, uint32 size)
{
	intmask	mask;			/* Saved interrupt mask		*/
	int32 res = SYSERR;

	mask = disable();

	/* Enqueue the data */
	res = enqueue_publish_data(topic, data, size);
	if(res != OK){
		restore(mask);
		return SYSERR;
	}

	restore(mask);
	return OK;
}
