#ifndef _RESOURCES_MONITOR_H
#define _RESOURCES_MONITOR_H

#define METRIC_NAME_1 "resources_usage"

typedef struct resources_stats_t {
	unsigned long counter;
	unsigned long MemTotal;
	unsigned long SwapTotal;
	unsigned long pid_VmRSS, pid_VmSwap;
	float CPU_usage_rate,  max_CPU_usage_rate, min_CPU_usage_rate; long long int accum_CPU_usage_rate;
	float RAM_usage_rate,  max_RAM_usage_rate, min_RAM_usage_rate; long long int accum_RAM_usage_rate;
	float swap_usage_rate, max_swap_usage_rate, min_swap_usage_rate; long long int accum_swap_usage_rate;
	unsigned long long process_CPU_time, before_process_CPU_time;
	unsigned long long total_cpu_time, before_total_cpu_time;
	unsigned long long before_accum_write_bytes;
	unsigned long long write_bytes, max_write_bytes, min_write_bytes, accum_write_bytes;
	unsigned long long before_accum_read_bytes;
	unsigned long long read_bytes,  max_read_bytes, min_read_bytes, accum_read_bytes;
// 	unsigned long long total_io_bytes;
	
	unsigned long long rcv_bytes, max_rcv_bytes, min_rcv_bytes , accum_rcv_bytes;
	unsigned long long send_bytes, max_send_bytes, min_send_bytes, accum_send_bytes ;
	float throughput,  max_throughput, min_throughput;
// 	unsigned long long total_net_bytes;
} resources_stats;

// ************** for monitoring the threads
struct system_data{
	int MemTotal;
	int MemFree;
	int Buffers;
	int Cached;
	int usedmemory;
	float cpu_system_load;
};

struct sub_task_user_def { //ddefned at add_tid_to_report
	char component_name[50];
	unsigned int pspid, pstid;
};

struct sub_task_data { // filled by stats_sample, procesa_pid_load, ...
	char taskid[50];
	unsigned int currentcore, pspid, pstid;
	float pmem, pcpu;
	int updated;
	long long int finishtime,totaltime,starttime;
	long long int rchar, wchar, syscr, syscw, read_bytes, write_bytes,cancelled_write_bytes;
	
	
	
	unsigned long counter;
	unsigned long MemTotal;
	unsigned long SwapTotal;
	unsigned long pid_VmRSS, pid_VmSwap;
	float CPU_usage_rate,  max_CPU_usage_rate, min_CPU_usage_rate; long long int accum_CPU_usage_rate;
	float RAM_usage_rate,  max_RAM_usage_rate, min_RAM_usage_rate; long long int accum_RAM_usage_rate;
	float swap_usage_rate, max_swap_usage_rate, min_swap_usage_rate; long long int accum_swap_usage_rate;
	unsigned long long process_CPU_time, before_process_CPU_time;
	unsigned long long total_cpu_time, before_total_cpu_time;
	unsigned long long max_write_bytes, min_write_bytes, accum_write_bytes,before_accum_write_bytes;
	unsigned long long max_read_bytes, min_read_bytes, accum_read_bytes,before_accum_read_bytes;
// 	unsigned long long total_io_bytes;
	
	unsigned long long rcv_bytes, max_rcv_bytes, min_rcv_bytes , accum_rcv_bytes;
	unsigned long long send_bytes, max_send_bytes, min_send_bytes, accum_send_bytes ;
	float throughput,  max_throughput, min_throughput;
// 	unsigned long long total_net_bytes;
};

struct cores_data {
	float total_load_core;
};

#define max_report_tids 10

typedef struct task_data_t {
// 	unsigned int tids[max_report_tids];
	unsigned long int microsleep;
	int pid;
	int maxprocesses, maxcores;
	unsigned int maxtotaltid;
	unsigned int totaltid;//counts the total of subtasks
	struct sub_task_data **subtask;  //for the subtasks running
	unsigned int total_user_def; //counts the total of user_def params
	struct sub_task_user_def **task_def;  //for the of user_def params
	struct cores_data *cores;  //for the total of cores running
	float total_load_cpu, totalpmem;
	long long int first_start,last_end;
}task_data;

struct disk_data{
	char labelstr[250];
	long long int rd_sectors;
	long long int wr_sectors;
	long long int rd_ios;
	long long int wr_ios;
};

// FUNCTIONS
char *save_stats_resources( struct resources_stats_t *stat, int pretty, int tabs);

char *save_stats_resources_comp( struct  sub_task_data *subtask, int pretty, int tabs);
	
// int printf_stats_resources(FILE *fp, struct resources_stats_t *stat, int pretty, int tabs);
// int printf_stats_resources_comp(FILE *fp, struct sub_task_data *subtask, int pretty, int tabs);
struct resources_stats_t *linux_resources(int pid, char *DataPath, long sampling_interval);

// FUNCTIONS // ************** for monitoring the threads
unsigned int print_stats(int searchprocess, task_data *my_task_data);

void stats_sample(const unsigned int pids, task_data *my_task_data);

void free_mem_report(struct task_data_t *my_task_data_a);
		
void init_stats(task_data *my_task_data_a);
unsigned int save_stats(FILE *fp, int searchprocess, task_data *my_task_data);
#endif
