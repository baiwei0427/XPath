#include<stdio.h>
#include<stdlib.h>
#include"Hash.h"

int main()
{
	struct RuleTable rt;
	Init_Table(&rt);
	struct Rule r1;
	struct Rule r2;
	struct Rule r3;
	r1.src_ip=0;
	r1.dst_ip=1;
	r1.protocol=6;
	r1.src_port=20;
	r1.dst_port=21;
	r1.new_ip=4;
	r1.mark=18;

	r2.src_ip=1;
	r2.dst_ip=2;
	r2.protocol=7;
	r2.src_port=21;
	r2.dst_port=22;
	r2.new_ip=5;
	r2.mark=19;

	r3.src_ip=3;
	r3.dst_ip=1;
	r3.protocol=2;
	r3.src_port=21;
	r3.dst_port=20;
	r3.new_ip=5;
	r3.mark=21;

	Insert_Table(&rt,&r1);
	Insert_Table(&rt,&r1);
	Insert_Table(&rt,&r2);

	Print_Table(&rt);

	Delete_Table(&rt,&r1);
	Delete_Table(&rt,&r1);
	Delete_Table(&rt,&r1);
	Delete_Table(&rt,&r3);
	Delete_Table(&rt,&r2);

	Insert_Table(&rt,&r3);
	Insert_Table(&rt,&r2);
	Print_Table(&rt);

	Empty_Table(&rt);
	
	return 0;
}