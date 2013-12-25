#ifndef HASH_H
#define HASH_H

#define HASH_RANGE 256 
#define QUEUE_SIZE 256

struct Rule{
	//Following 5 tuples defines a flow
	unsigned int src_ip;   //souce ip address 
	unsigned int dst_ip;   //destination ip address
	unsigned int protocol; //ip_header->protocol tcp:6 udp:17 icmp:1
	unsigned int src_port; //source tcp/udp port
	unsigned int dst_port; //destination tcp/udp port

	//Following 2 tuples defines an action
	unsigned int new_ip;   //new ip address need to be transfered to
	unsigned int mark;     //new mark
};

//Link Node of Rule
struct RuleNode{
	struct Rule r;         //content of rule
	struct RuleNode* next; //pointer to next node 
};

//Link List of Rules
struct RuleList{
	struct RuleNode* head; //pointer to head node of this link list
	int len;               //current length of this list (max: QUEUE_SIZE)
};

//Hash Table of Rules
struct RuleTable{
	struct RuleList* table; //many RuleList (HASH_RANGE)
	int size;               //total number of nodes in this table
};

//Hash function, calculate the rule should be inserted into which RuleList
static unsigned int Hash(struct Rule* r)
{
	//return a value in [0,HASH_RANGE-1]
	return 	(r->src_ip%(256*256*256))*(r->dst_ip%(256*256*256)*r->dst_port%HASH_RANGE);
}

//Determine whether two Rules are equal 
static int Equal(struct Rule* r1,struct Rule* r2)
{
	return (r1->src_ip==r2->src_ip)&&(r1->dst_ip==r2->dst_ip)&&(r1->protocol==r2->protocol)&&(r1->src_port==r2->src_port)&&(r1->dst_port==r2->dst_port);
}

//Initialize a RuleNode
static void Init_Node(struct RuleNode* rn)
{
	//Initialize all values as zero or null
	rn->r.src_ip=0;
	rn->r.dst_ip=0;
	rn->r.protocol=0;
	rn->r.src_port=0;
	rn->r.dst_port=0;
	rn->r.new_ip=0;
	rn->r.mark=0;
	rn->next=NULL;
}

//Initialize a RuleList
static void Init_List(struct RuleList* rl)
{
	//No nodes in current list
	rl->len=0;
	rl->head=(struct RuleNode*)malloc(sizeof(struct RuleNode));
	//Initialize the header node of this RuleList as a null node
	Init_Node(rl->head);
}

//Initialize a RuleTable
static void Init_Table(struct RuleTable* rt)
{
	int i=0;
	//allocate space for RuleLists
	rt->table=(struct RuleList*)malloc(HASH_RANGE*sizeof(struct RuleList));
	for(i=0;i<HASH_RANGE;i++)
	{
		//Initialize each RuleList
		Init_List(&(rt->table[i]));
	}
	//No nodes in current table
	rt->size=0;
}

//Insert a Rule into a RuleList
static void Insert_List(struct RuleList* rl, struct Rule* r)
{
	if(rl->len>=QUEUE_SIZE) {

		printf("No enough space in this RuleList\n");
		return;

	} else {
		
		struct RuleNode* tmp=rl->head;

		//Find the tail of this RuleList
		while(1)
		{
			//If pointer to next node is NULL, we find the tail of this RuleList
			if(tmp->next==NULL)
			{
				//Allocate space for new RuleNode
				tmp->next=(struct RuleNode*)malloc(sizeof(struct RuleNode));
				//Copy data for this new RuleNode
				tmp->next->r=*r;
				//Pointer to next RuleNode is NUll
				tmp->next->next=NULL;
				//Increase length of RuleList
				rl->len++;
				return;
			}
			else
			{
				tmp=tmp->next;
			}
		}
	}
}

//Insert a rule to RuleTable
static void Insert_Table(struct RuleTable* rt,struct Rule* r)
{
	unsigned int index=0;
	index=Hash(r);
	//Insert Rule to appropriate RuleList based on Hash value
	Insert_List(&(rt->table[index]),r);
}

//Delete a rule from RuleList
static void Delete_List(struct RuleList* rl, struct Rule* r)
{
	if(rl->len==0) {

		//printf("No RuleNode in this RuleList to delete\n");
		return;

	} else {
		
		struct RuleNode* tmp=rl->head;

		//Find the tail of this RuleList
		while(1)
		{
			//If pointer to next node is NULL, we find the tail of this RuleList, no more RuleNodes
			if(tmp->next==NULL)
			{
				return;
			}
			//Find matching rule (matching RuleNode is tmp->next rather than tmp)
			if(Equal(&(tmp->next->r),r))//tmp->next->r==(*r))
			{
				struct RuleNode* s=tmp->next;
				tmp->next=s->next;
				//Delete matching RuleNode from this RuleList
				free(s);
				rl->len--;
				return;
			}
			else
			{
				tmp=tmp->next;
			}
		}
	}
}

static void Delete_Table(struct RuleTable* rt,struct Rule* r)
{
	unsigned int index=0;
	index=Hash(r);
	//Delete Rule from appropriate RuleList based on Hash value
	Delete_List(&(rt->table[index]),r);
}

static void Empty_List(struct RuleList* rl)
{
	struct RuleNode* NextNode;
	struct RuleNode* Ptr;
	for(Ptr=rl->head->next;Ptr!=NULL;Ptr=NextNode)
	{
		NextNode=Ptr->next;
		free(Ptr);
	}
	rl->head=NULL;

}

static void Empty_Table(struct RuleTable* rt)
{
	int i=0;
	for(i=0;i<HASH_RANGE;i++)
	{
		Empty_List(&(rt->table[i]));
	}
	free(rt->table);
}

static void Print_Node(struct RuleNode* rn)
{
	printf("src_ip:%d dst_ip:%d src_port:%d dst_port:%d protocol:%d\n",rn->r.src_ip,rn->r.dst_ip,rn->r.src_port,rn->r.dst_port,rn->r.protocol);
}

static void Print_List(struct RuleList* rl)
{
	struct RuleNode* Ptr;
	for(Ptr=rl->head->next;Ptr!=NULL;Ptr=Ptr->next)
	{
		Print_Node(Ptr);
	}
}

static void Print_Table(struct RuleTable* rt)
{
	int i=0;
	for(i=0;i<HASH_RANGE;i++)
	{
		if(rt->table[i].len>0)
		{
			printf("RuleList %d\n",i);
			Print_List(&(rt->table[i]));
			printf("\n");
		}
	}
}

#endif