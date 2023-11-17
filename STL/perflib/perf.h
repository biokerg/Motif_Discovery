

#ifndef __PERFLIB_H__
#define __PERFLIB_H__

#include <perfmon/pfmlib.h>
#include <perfmon/pfmlib_perf_event.h>
#include <sys/prctl.h>
#include <string.h>
#include <err.h>
#include <malloc.h>
#include <fstream>
#include <string>
#include <vector>

#define PERF_FORMAT_SCALE (PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING)
#define HARDWARE_COUNTERS	4
//using namespace std;

class perf
{

private:

	int splitstring(std::string str, std::string*& list)
	{
		int num=0;
		size_t pos2=0;
		while(pos2<str.length())
		{
			int next=str.find(',',pos2);
			pos2=(next>0)?next+1:str.length();
			num++;
		}
		list=new std::string[num];
		int index=0;
		size_t pos1=0;
		while(pos1<str.length())
		{
			int next=str.find(',',pos1);
			pos2=(next>0)?next:str.length();
			list[index++]=str.substr(pos1,pos2-pos1);
			pos1=pos2+1;
		}
		return num;
	}

	typedef struct 
	{
		struct perf_event_attr hw;
		uint64_t values[3];
		uint64_t prev_values[3];
		uint64_t delta_prev_values[3];
		char *name;
		uint64_t id; /* event id kernel */
		void *buf;
		size_t pgmsk;
		int group_leader;
		int fd;
		int max_fds;
		int idx; /* opaque libpfm event identifier */
		char *fstr; /* fstr from library, must be freed */
	} perf_event_desc_t;
	
	perf_event_desc_t *fds;
	int num_fds;
	bool started;
	char livestring[32];
	int _pid;
	int _core;
	uint64_t* last_values;
	std::string* eventlist;
	
	/*static inline uint64_t	perf_scale(uint64_t *values)
	{
		uint64_t res = 0;

		if (!values[2] && !values[1] && values[0])
			warnx("WARNING: time_running = 0 = time_enabled, raw count not zero\n");

		if (values[2] > values[1])
			warnx("WARNING: time_running > time_enabled\n");

		if (values[2])
			res = (uint64_t)((double)values[0] * values[1]/values[2]);
		return res;
	}*/

	static inline uint64_t scale(uint64_t *new_values, uint64_t *prev_values)
	{
		if(new_values[2]==0)
		{//no change in running time
			return (uint64_t)((double)new_values[0] * (double)new_values[1]/(double)prev_values[1]);
		}
		else
		{//change in running time
			return (uint64_t)((double)new_values[0] * (double)new_values[1]/(double)new_values[2]);
		}
	}

	bool create_handle(std::string event ,perf_event_desc_t* fd)
	{
		pfm_perf_encode_arg_t arg;
		memset(fd, 0, sizeof(*fd));
		memset(&arg, 0, sizeof(arg));
		arg.fstr = &fd->fstr;
		arg.size=sizeof(pfm_perf_encode_arg_t);
		arg.attr = &fd->hw;
		
		int ret =pfm_get_os_event_encoding(event.c_str(),PFM_PLM0|PFM_PLM3,PFM_OS_PERF_EVENT_EXT,&arg);
		if (ret == PFM_SUCCESS) 
		{
			fd->name = strdup(*arg.fstr);
			fd->group_leader = 0;
			fd->idx = arg.idx;
			return true;
		}
		else
			return false;
	}

public:	

	
	perf(){}
	
	static void pfminit()
	{
		int ret=pfm_initialize();
		if (ret != PFM_SUCCESS)
			errx(1, "cannot initialize libpfm library: %s", pfm_strerror(ret));
	}
		
	void init()
	{
		fds=0;
		num_fds=0;
		started=false;
	}
	
	int setup(std::string events, std::vector<int> pinnedList={})
	{	
		num_fds=splitstring(events, eventlist);	
		if(fds)
			delete fds;
		fds=new perf_event_desc_t[num_fds];
		int j=0;
		int pinnedCount=0;
		for (int i=0;i<num_fds;i++)
		{			
			if(!create_handle(eventlist[i],&fds[i]))
			{
				printf("\nfail event name:%s\n",eventlist[i].c_str());
				delete fds;
				fds=0;
				exit(0);
				return 0;
			}
			else
			{
				for(int j=0; j<3; j++)
				{
					fds[i].prev_values[j] = 0;
					fds[i].delta_prev_values[j] = 0;
				}
				fds[i].hw.read_format = PERF_FORMAT_SCALE;
				if(num_fds<=HARDWARE_COUNTERS)
				{
					fds[i].hw.pinned = true;
					pinnedCount++;
				}
				else
				{
					fds[i].hw.pinned = false;
					for (int j = 0; j < pinnedList.size(); j++)
					{
						if (pinnedList[j] == i && pinnedCount<HARDWARE_COUNTERS-1)
						{
							fds[i].hw.pinned = true;
							pinnedCount++;
						}
					}
				}
			}
		}
		last_values=new uint64_t[num_fds];
		return num_fds;
	}	
	
	bool ready()
	{
		if(fds)
			return true;
		else
			return false;
	}

	void start()
	{
	//tracks the current process on any core
		start_pid_core(0,-1);
	}
	
	void start_pid(int pid)
	{
	//tracks the specified process on any core
		start_pid_core(pid,-1);
	}
	
	void start_core(int core)
	{
	//tracks the specified core activity
		start_pid_core(-1,core);
	}
	
	void start_pid_core(int pid,int core)
	{
	//pid=-1 any process, pid=0 this process, pid>0 specified process
	//core=-1 any core, core>=0 specified core
		_pid=pid;
		_core=core;
		for(int i=0; i < num_fds; i++) 
		{
			memset(fds[i].prev_values,0,sizeof(uint64_t)*3);
			fds[i].hw.sample_period=0; // this line specifies that the event is a counting event and is not a sampling event
			/* each event is in an independent group (multiplexing likely) */
			fds[i].fd = perf_event_open(&fds[i].hw, pid, core, -1, 0);
			if (fds[i].fd == -1)
				err(1, "cannot open event: %s\nyou may need to be root for correct execution!", fds[i].name);
		}
		
		sprintf(livestring, "/proc/%d/status", pid);
		started=true;			
	}
	
	void stop()
	{
		read(last_values);
		for (int i = 0; i < num_fds; i++)
		{
	  		close(fds[i].fd);
  		}
		//perf_free_fds(fds, num_fds);
		started=false;
	}
	
	int read(uint64_t out[])
	{
		uint64_t values[3];
		uint64_t delta_values[3];
		ssize_t ret=0;
		for (int i = 0; i < num_fds; i++) 
		{
			if(started)
			{
				::read(fds[i].fd, values, sizeof(uint64_t)*3);
				for(int j=0; j<3; j++)
					delta_values[j]=values[j]-fds[i].prev_values[j];
				if(fds[i].hw.pinned)
					out[i] = delta_values[0];
				else
					out[i] = scale(delta_values, fds[i].delta_prev_values);
				for(int j=0; j<3; j++)
				{
					fds[i].prev_values[j] = values[j];
					fds[i].delta_prev_values[j] = delta_values[j];
				}
			}
			else
			{
				out[i]=last_values[i];
			}			
		}
		return 1;
	}
	
	bool alive()
	{
		return (access(livestring, F_OK) == 0);
	}
	
	int get_pid()
	{
		return _pid;
	}
	
	void pmus()
	{
		pfm_pmu_info_t pinfo;
		memset(&pinfo, 0, sizeof(pinfo));
		int total_supported_events=0;
		printf("\nDetected PMU models:%d\n",PFM_PMU_MAX);
		for(int i=0; i < PFM_PMU_MAX; i++) 
		{
			int ret = pfm_get_pmu_info((pfm_pmu_t)i, &pinfo);
			if (ret != PFM_SUCCESS)
				continue;
			if (pinfo.is_present) 
			{
				printf("\t[code: %d, name: %s, description: \"%s\"]\n", i, pinfo.name, pinfo.desc);
				total_supported_events += pinfo.nevents;
			}
		}
		printf("\tTotal supported events: %d\n",total_supported_events);
	}
	
	void list()
	{
		pfm_event_info_t info;
		pfm_pmu_info_t pinfo;
		memset(&info, 0, sizeof(info));
		memset(&pinfo, 0, sizeof(pinfo));
		printf("\nDetected EVENTS:\n\t%-5s %-50s %-10s %-10s\n------------------------------\n","Row", "Event Name","Event ID","PMU Code");
		int c=0;
		for(int i=0; i < PFM_PMU_MAX; i++) 
		{
			int ret = pfm_get_pmu_info((pfm_pmu_t)i, &pinfo);
			if (ret != PFM_SUCCESS)
				continue;
			if (pinfo.is_present) 
			{
				for (int j = pinfo.first_event; j != -1; j= pfm_get_event_next(j)) 
				{
					ret = pfm_get_event_info(j,PFM_OS_NONE, &info);
					if (ret != PFM_SUCCESS)
						errx(1, "cannot get event info");
					c++;
					printf("\t%-5d %-50s %-10d %-10d\n\tDescription: %s\n\t---------------\n",c, info.name,j,i,info.desc);
				}
			}
		}
	}
};

class PerfActivator
{
public:
	PerfActivator()
	{
		perf::pfminit();
	}
}_____PerfActivatorInstance;

#endif //__PERFLIB_H__


