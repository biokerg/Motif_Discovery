#include <iostream>
using namespace std;
#include <vector>

#include <reporting.h>

int main()
{
	int b[]={10,25};
	reporting rp;
	rp.init("test");
	rp.insertln(b,2);
	double c=1,d=2,e=3;
	rp.insertln(c,{d,e});
	return 0;
}

