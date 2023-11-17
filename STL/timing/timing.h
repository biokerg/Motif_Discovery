//TimingLib ver 1.0 Copyright (c) Morteza Moradi (moradi.edu@gmail.com), all rights reserved.

#ifndef TIMING_H_
#define TIMING_H_

#include <time.h>
#include <math.h>
#include <string>
//using namespace std;

inline unsigned long long CLOCK()
{
	int ttime=0;
	struct timespec	FullTime;
	clock_gettime(CLOCK_MONOTONIC,&FullTime);
	unsigned long long t=(unsigned long)((unsigned long long) FullTime.tv_nsec + (unsigned long long)FullTime.tv_sec * 1000000000);
	return t;
}

std::string DATETIME()
{
	char buff[50]="";
	time_t now=time(0);
	strftime(buff, sizeof(buff), "%Y-%m-%d %X", localtime(&now));
	return buff;
}

#define CLOCKSEC($1)	(double)((long)$1/1000)/1000
#define ElapsSec($1)	(unsigned long )(((long)(CLOCK()-$1)/1000)/1000)

class timing
{
private:
	unsigned long long t1;
	unsigned long long t2;
	
	unsigned long long total;
	
	bool counting;
	
public:
	
	timing()
	{
		init();
	}
	
	void init()
	{
		counting=false;
		total=0;
	}
	
	std::string time()
	{
		char buff[20]="";
		time_t now=::time(0);
		strftime(buff, sizeof(buff), "%X", localtime(&now));
		std::string str=buff;
		return str;
	}

	void start()
	{
		counting=true;
		t1=CLOCK();
	}
	
	void stop()
	{
		t2=CLOCK();
		counting=false;
		total+=t2-t1;
	}
	
	unsigned long long nano()
	{
		if(counting)
			return CLOCK()-t1;
		else
			return t2-t1;
	}
	
	unsigned long long micro()
	{
		if(counting)
			return (double)(CLOCK()-t1)/1000;
		else
			return (double)(t2-t1)/1000;
	}
	
	unsigned long long mili()
	{
		if(counting)
			return (double)(CLOCK()-t1)/1000000;
		else
			return (double)(t2-t1)/1000000;
	}
	
	double sec()
	{
		if(counting)
		
			return (double)(CLOCK()-t1)/1000000000;
		else
		
			return (double)(t2-t1)/1000000000;
	}
	
	unsigned long long totalnano()
	{
		if(counting)
			return (CLOCK()+total-t1);
		else
			return total;
	}
	
	unsigned long long totalmicro()
	{
		if(counting)
			return (CLOCK()+total-t1)/1000;
		else
			return total/1000;
	}
	
	double totalmili()
	{
		if(counting)
			return (double)(CLOCK()+total-t1)/1000000;
		else
			return (double)total/1000000;
	}
	
	double totalsec()
	{
		if(counting)
		
			return (double)(CLOCK()+total-t1)/1000000000;
		else
		
			return (double)total/1000000000;
	}

	void reset()
	{
		t1=CLOCK();
	}
};

#endif /* TIMING_H_ */
