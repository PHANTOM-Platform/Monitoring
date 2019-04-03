#ifndef _RESOURCES_MONITOR_H
#define _RESOURCES_MONITOR_H

#include "mf_api.h"

#define METRIC_NAME_1 "resources_usage"

#define NAME_LENGTH          32

struct cores_data {
	float total_load_core;
	float total_joules_core;
	long long int time_of_last_measured;
	long long int time_between_measures;
	float total_watts_core;
	long int core_freq;
};

typedef struct resources_stats_t {
	unsigned long counter;
	unsigned long MemTotal;
	unsigned long SwapTotal;
	unsigned long pid_VmRSS, pid_VmSwap;
	float CPU_usage_rate,  max_CPU_usage_rate, min_CPU_usage_rate; long long int accum_CPU_usage_rate;
	float RAM_usage_rate,  max_RAM_usage_rate, min_RAM_usage_rate; long long int accum_RAM_usage_rate;
	float swap_usage_rate, max_swap_usage_rate, min_swap_usage_rate; long long int accum_swap_usage_rate;
	unsigned long long pid_runtime, accum_pid_runtime, before_accum_pid_runtime;
	unsigned long long total_cpu_time, before_total_cpu_time;
	unsigned long long before_accum_write_bytes;
	unsigned long long write_bytes, max_write_bytes, min_write_bytes, accum_write_bytes;
	unsigned long long before_accum_read_bytes;
	unsigned long long read_bytes,  max_read_bytes, min_read_bytes, accum_read_bytes;
	unsigned long long cancelled_writes, accum_cancelled_writes, before_accum_cancelled_writes;
// 	unsigned long long total_io_bytes;
	unsigned long long rcv_bytes, max_rcv_bytes, min_rcv_bytes , accum_rcv_bytes;
	unsigned long long send_bytes, max_send_bytes, min_send_bytes, accum_send_bytes ;
	float throughput,  max_throughput, min_throughput;
	
//from linux_sys_power
	unsigned long long sys_itv, accum_sys_itv, before_accum_sys_itv;
	float sys_runtime, accum_sys_runtime, before_accum_sys_runtime ;//counts in seconds amount of time
	unsigned long long pid_l2_cache_misses, pid_accum_l2_cache_misses, before_pid_accum_l2_cache_misses;
	float sys_cpu_energy;
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

struct sub_task_user_def { //defined at add_tid_to_report
	char component_name[50];
	unsigned int pspid, pstid;
};

struct sub_task_data { // filled by stats_sample, procesa_pid_load, ...
	char taskid[50];
	unsigned int currentcore, pspid, pstid;
	float pmem, pcpu, total_cpu_energy;

	long long unsigned int start_comp;
	unsigned long long int time_of_last_measured;
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
	unsigned long long pid_runtime, before_pid_runtime;
	unsigned long long total_cpu_time, before_total_cpu_time;
	unsigned long long max_write_bytes, min_write_bytes, accum_write_bytes,before_accum_write_bytes;
	unsigned long long max_read_bytes, min_read_bytes, accum_read_bytes,before_accum_read_bytes;
	unsigned long long cancelled_writes ;
// 	unsigned long long total_io_bytes;
	unsigned long long rcv_bytes, max_rcv_bytes, min_rcv_bytes , accum_rcv_bytes;
	unsigned long long send_bytes, max_send_bytes, min_send_bytes, accum_send_bytes ;
	float throughput,  max_throughput, min_throughput;
// 	unsigned long long total_net_bytes;
};


struct disk_data{
	char labelstr[250];
	long long int rd_sectors;
	long long int wr_sectors;
	long long int rd_ios;
	long long int wr_ios;
};


typedef struct energy_model_t {
	//CPU Energy Model:
// 	float cpu_factor_c ;//= 11.2; // watts on idle state
// 	float cpu_factor_k ;//= 50;  //= (Energy_2 -Energ_1)/(Freq_2^3 - Freq_1^3) = (90- 40)/(4^3-3^3)=50  <-- freq in GHz
	unsigned long long freq_min;//400000 KHz
	unsigned long long freq_max;//2800000 KHz
	float MAX_CPU_POWER;//24.5;//[0]<--- per core
	float MIN_CPU_POWER;//6.0;//[1]  <--- per core
	//MEM Energy Model:
	float L2CACHE_LINE_SIZE;//=128;
	float L2CACHE_MISS_LATENCY;//=59.80;
	float MEMORY_POWER;//=2.016;
// 		values are such:
// 		DDR1 RAM (2.5 Volts) 4 to 5.5 W (depending of the clock freq)
// 		DDR2 RAM (1.8 Volts) 3 to 4.5 W
// 		DDR3 RAM (1.5 Volts) 2 to 3 W
	float sata_drive;//=15.0;
// 		values are such:
// 		SATA DVD Drive 15 to 27 W
// 		SATA Blu-ray Drive 25 to 30 W
	float case_fan;// = 1;
// 		values are such:
// 		80 mm Case Fan (2,000 RPM) 0.6 to 1.8 W
// 		80 mm Case Fan (3,000 RPM) 2.4 to 3 W
// 		120 mm Case Fan (1,200 RPM) 0.6 to 2.3 W
// 		120 mm Case Fan (2,000 RPM) 3.6 to 6 W
// 		140 mm Case Fan (1,000 RPM) 0.9 to 1.7 W
// 		140 mm Case Fan (2,000 RPM) 4.2 to 6 W
	float hd_power;// = 8;
// 		values are such:
// 		Solid State Drive SSD 0.6 to 2.8 W
// 		2.5" Hard Disk Drive HDD 0.7 to 3 W
// 		3.5" Hard Disk Drive HDD 6.5 to 9 W
	float E_DISK_R_PER_KB;//=0.0556;
	float E_DISK_W_PER_KB;//=0.0438;
	float E_NET_SND_PER_MB;//=0.14256387;
	float E_NET_RCV_PER_MB;//=0.24133936;

	float motherboard_power;// = 40;
// 		values are such:
// 		Regular Motherboard 25 to 40 W
// 		High End Motherboard 45 to 80 W
}energy_model;

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
int CPU_stat_process_read(int pid, struct resources_stats_t *stats_now);
int io_stats_read(int pid, struct resources_stats_t *stats_now);
int CPU_stat_read(struct resources_stats_t *stats_now, const float ticksPerSecond);

unsigned int procesa_pid_load(int pid, unsigned int argmaxcores, struct task_data_t *my_task_data, energy_model param_energy);
// unsigned int procesa_pid_load_power(int pid, unsigned int argmaxcores, struct task_data_t *my_task_data, energy_model param_energy);
void procesa_task_io(task_data *my_task_data );

size_t execute_command(const char *command, char *comout, size_t *comalloc);
unsigned int process_str(char *input, char *output, unsigned const int start, const unsigned int max_output_size);
unsigned int getline_str(char *input, char *output, unsigned const int start);
unsigned int find_str(int start, const char source[], const char cad1[]);
int remove_str(int start ,char source[], const char cadenaBuscar[]);
int find_llint_from_label(char *loadstr, const char *label, long long int *to_update);



#endif
