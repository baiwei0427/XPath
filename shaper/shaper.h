#ifndef SHAPER_H
#define SHAPER_H

#include <linux/vmalloc.h>

#define QUEUE_SIZE 2048

struct Packet{

    int (*okfn)(struct sk_buff *);  //function pointer to reinject packets
    struct sk_buff *skb;           //socket buffer pointer to packet               
}; 

struct PacketQueue{
    struct Packet *packets;
    int head;
	int size;
};

void Init_PacketQueue(struct PacketQueue* q)
{
	q->packets=vmalloc(QUEUE_SIZE*sizeof(struct Packet));
	q->head=0;
	q->size=0;
}

void Free_PacketQueue(struct PacketQueue* q)
{
	vfree(q->packets);
}

int Enqueue_PacketQueue(struct PacketQueue* q,struct sk_buff *skb,int (*okfn)(struct sk_buff *))
{
	//There is capacity to contain new packets
	if(q->size<QUEUE_SIZE) {

		//Index for new insert packet
		int queueIndex=(q->head+q->size)%QUEUE_SIZE;
		q->packets[queueIndex].skb=skb;
		q->packets[queueIndex].okfn=okfn;
		q->size++;
		return 1;

	} else {

		return 0;
	}
}

int Dequeue_PacketQueue(struct PacketQueue* q)
{
	if(q->size>0) {
		//Reinject head packet of current queue
		(q->packets[q->head].okfn)(q->packets[q->head].skb);
		q->size--;
		q->head=(q->head+1)%QUEUE_SIZE;
		return 1;

	} else {
	
		return 0;
	}

}

#endif