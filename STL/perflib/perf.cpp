#include <iostream>
#include "perf.h"
using namespace std;
int main()
{
	perf p1,p2;
	p1.init();
	p1.setup("cycles,instructions,L1-DCACHE-LOAD-MISSES,L1-DCACHE-STORE-MISSES,cache-misses\0",5);
	//printf("ready=%s\n",p1.ready()?"yes":"no");		
	p1.start_core(0);
	
	p2.init();
	p2.setup("cycles,instructions,cache-misses\0",3);
	//printf("ready=%s\n",p2.ready()?"yes":"no");	
	p2.start_core(1);
	
	uint64_t a[5];
	for(int i=0;;i++)
	{
		p1.read(a);
		int b=a[0];
		int c=a[1];
		int d=a[2];
		int e=a[3];
		int f=a[4];
		printf("core 0 inst=%d cycl=%d IPC=%f l1-cache-misses=%d\n",c,b,(double)c/(double)b,d+e);
		
		p2.read(a);
		b=a[0];
		c=a[1];
		d=a[2];
		printf("core 1 inst=%d cycl=%d IPC=%f cache-misses=%d\n------------------\n",c,b,(double)c/(double)b,d);
		sleep(1);
	}
	p1.stop();
	return 0;
}
