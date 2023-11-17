#include <iostream>
#include <string>
#include "perf.h"
using namespace std;
int main()
{
	int core;
	cout<<"Please enter the core number to monitor: ";
	cin >> core;
	perf p;
    p.init();
    string ctrs="cycles,instructions,llc_references,llc_misses";//,l1-dcache-stores";
	int num=p.setup(ctrs);
	p.start_core(core);
	uint64_t *a=new uint64_t[num];
	cout<<ctrs<<endl;
    while(true)
    {
		usleep(1000000);
		
		p.read(a);
		for(int j=0;j<num;j++)
		cout<<a[j]<<",";
		cout<<endl;
	}
	p.stop();
	
	return 0;
}
