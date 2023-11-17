

#ifndef __REPORTING_H__
#define __REPORTING_H__

#define MAX_INDEX 1000

#include <string.h>
#include <fstream>
#include <unistd.h>
#include <threading.h>
#include <timing.h>
#include <stdarg.h>
#include <vector>

//using namespace std;
	
class reporting
{
private:
	bool monitoring;
	bool startFinishLine;
	std::string file;
	std::string file_extention;
	THREAD_T monitor_thread_handler;
	char history[MAX_INDEX];
	int history_index;    
	int monitor_sleeptime;
	typedef void (*READFUNC)(reporting*);
	READFUNC readfn;
	bool firstOfLine;
	LOCK_T _lock;
	timing tim;
	
	static void* monitor_thread(void* p)
	{
		reporting* rp=(reporting*)p;
		int _sleeptime=rp->monitor_sleeptime;
		while(rp->monitoring)
		{
			rp->readfn(rp);
			usleep(_sleeptime);
		}
		rp->flush();
		EXIT();
	}	
	
	void monitor_finish()
	{
		if(startFinishLine)
			write(file + " finished at "+ DATETIME()+" ,"+std::to_string(tim.sec())+", seconds\n");
		flush();
	}
	
	inline void makebuff(char* buff,int a)
	{
		sprintf(buff,"%d",a);
	}
	
	inline void makebuff(char* buff,double a)
	{
		sprintf(buff,"%f",a);
	}
	
	inline void makebuff(char* buff,long long unsigned int a)
	{
		sprintf(buff,"%llu",a);
	}
	
	inline void makebuff(char* buff,long unsigned int a)
	{
		sprintf(buff,"%lu",a);
	}
	
	inline void makebuff(char* buff,long long int a)
	{
		sprintf(buff,"%llu",a);
	}
	
	inline void makebuff(char* buff,long int a)
	{
		sprintf(buff,"%lu",a);
	}
	
	inline void makebuff(char* buff,std::string a)
	{
		strcpy(buff,a.c_str());
	}
	
public:	
	void* tag;
	
	reporting(){}
	
	~reporting()
	{ 
		flush();
	}
	
	void init(std::string _file)
	{
		file=_file;
		file_extention=".csv";
		monitoring=false;
		history_index=0;
		monitor_sleeptime=1000000;
		firstOfLine=true;
		startFinishLine=false;
		LOCK_INIT(&_lock);
	}

	void addDateTime()
	{
		file_extention=DATETIME() + ".csv";
	}
	
	void addStartFinish()
	{
		startFinishLine=true;
	}
	
	void write(std::string value)
	{
	
		for(int i=0;value[i]!=0;i++)
		{
			if(history_index>MAX_INDEX-1)
			{
				flush();
			}
			history[history_index]=value[i];
			history_index++;						
		}
		firstOfLine=false;
	}
	
	void writeln(std::string value)
	{
		write(value);
		write("\r\n");
		firstOfLine=true;
	}
	
	void newLine()
	{
		writeln("");
	}
	
	void section(std::string sname)
	{
		if(!firstOfLine)
			newLine();
		writeln("@"+sname);
	}

	template<class T>
	void insertln(T* list,int n)
	{
		insert(list,n);
		write("\r\n");
		firstOfLine=true;
	}
	
	template<class T>
	void insert(T* list,int n)
	{
		char buff[30];
		if(!firstOfLine)
			write(",");
		for(int i=0;i<n;i++)
		{
			makebuff(buff,list[i]);
			write(buff);
			if(i<n-1)
				write(",");
		}
	}
	
	template<class T>
	void insertln(T value)
	{
		insertln(&value,1);
	}
	
	template<class T>
	void insert(T value)
	{
		insert(&value,1);
	}
	
	template<class T>
	void insert(T v, std::vector<T> l)
	{
		insert(v);
		for(int i=0; i<l.size(); i++)
		{
			insert(l[i]);
		}
	}
	
	template<class T>
	void insertln(T v, std::vector<T> l)
	{
		insertList(l);
		newLine();
	}
		
	void flush()
	{
		std::ofstream f;
		f.open(file + file_extention, std::ios_base::app);	
		for(int i=0;i<history_index;i++)
		{
			f<< history[i]; 
		}
		f.close();		
		history_index=0;
	}
	
	void start(READFUNC fn,int sleeptime=1000000)
	{
		readfn=fn;
		monitor_sleeptime=sleeptime;		
		monitoring=true;	
		if(startFinishLine)
			writeln(file+" started at "+ DATETIME());
		tim.start();
		SPAWN(&monitor_thread_handler,monitor_thread,this);
	}
	
	void stop()
	{
		if(monitoring)
		{
			tim.stop();
			monitoring=false;
			WAIT(monitor_thread_handler);
			monitor_finish();
		}
	}
	
	void lock()
	{
		LOCK(&_lock);
	}
	
	void unlock()
	{
		UNLOCK(&_lock);
	}
};

#endif //__REPORTING_H__


