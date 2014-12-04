#ifndef QUEUE_H
#define QUEUE_H

#define MAX_QUEUE_LEN 1024

struct Packet
{
	int (*okfn)(struct sk_buff *); //function pointer to reinject packets
	struct sk_buff *skb;                  //socket buffer pointer to packet   
};

struct PacketQueue
{
    struct Packet *packets; //Array of packets
    unsigned int head;         //Head offset
	unsigned int len;            //Current queue length
    spinlock_t queue_lock;//Lock for the PacketQueue
};

//flags should be GFP_ATOMIC or GFP_KERNEL 
static void Init_PacketQueue(struct PacketQueue* q, int flags)
{
	q->packets=kmalloc(MAX_QUEUE_LEN*sizeof(struct Packet), flags);
	q->head=0;
	q->len=0;
	spin_lock_init(&q->queue_lock);
}

static void Free_PacketQueue(struct PacketQueue* q)
{
	kfree(q->packets);
}

//Enqueue packet. If it succeeds, return 1
static int Enqueue_PacketQueue(struct PacketQueue* q,struct sk_buff *skb,int (*okfn)(struct sk_buff *))
{
    //unsigned long flags;                     
	
	//There is capacity to contain new packets
	if(q->len<MAX_QUEUE_LEN) 
    {
        //spin_lock_irqsave(&(q->queue_lock),flags);
		//Index for new insert packet
		int queueIndex=(q->head+q->len)%MAX_QUEUE_LEN;
		q->packets[queueIndex].skb=skb;
		q->packets[queueIndex].okfn=okfn;
		q->len++;
        //spin_unlock_irqrestore(&(q->queue_lock),flags);
		return 1;
	} 
    else
    {
		return 0;
	}
}

static int Dequeue_PacketQueue(struct PacketQueue* q)
{
    unsigned long flags;       
	
	if(q->len>0) 
    {
        //spin_lock_irqsave(&(q->queue_lock),flags);
		q->len--;
		//Dequeue packet
		(q->packets[q->head].okfn)(q->packets[q->head].skb);
		//Reinject head packet of current queue
		q->head=(q->head+1)%MAX_QUEUE_LEN;
        //spin_unlock_irqrestore(&(q->queue_lock),flags);
		return 1;
	} 
    else 
    {
		return 0;
	}
}

#endif