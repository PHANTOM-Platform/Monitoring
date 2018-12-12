#ifndef _POWER_MONITOR_H
#define _POWER_MONITOR_H

#define METRIC_NAME_3 "power"

/* CPU Specifications */
/***********************************************************************
CPU: in my laptop
- 800MHZ: 6W
- 2.Ghz: 24.5W
***********************************************************************/
//#define MAX_CPU_POWER 24.5
//#define MIN_CPU_POWER 6.0

/*
**********************************************************************
Memory Specification
**********************************************************************
*/
//#define MEMORY_POWER 2.016 //in Watts, from my memory module specification
//#define L2CACHE_MISS_LATENCY 59.80 //ns, get use calibrator
//#define L2CACHE_LINE_SIZE 128 //byte get use calibrator

/*
**********************************************************************
Energy, in milliJoul, when read a kilobytes
- Read: 0.02 * 2.78
- Write: 0.02 * 2.19
**********************************************************************
*/
//#define E_DISK_R_PER_KB (0.02 * 2.78)
//#define E_DISK_W_PER_KB (0.02 * 2.19)

typedef struct pid_stats_info_t {
	unsigned long long sys_itv;
	unsigned long long sys_runtime;
	unsigned long long pid_runtime;
	unsigned long long pid_read_bytes;
	unsigned long long pid_write_bytes;
	unsigned long long pid_cancelled_writes;
	unsigned long long pid_l2_cache_misses;
	float sys_cpu_energy;
} pid_stats_info;

int power_monitor(int pid, char *DataPath, long sampling_interval);

int create_perf_stat_counter(int pid);
int read_and_check(int fd, int pid, pid_stats_info *info);
int calculate_and_update(pid_stats_info *before, pid_stats_info *after, pid_stats_info *delta);
int read_pid_time(int pid, pid_stats_info *info);
int read_pid_io(int pid, pid_stats_info *info);
int read_sys_time(pid_stats_info *info);
int cpu_freq_stat(pid_stats_info *info);
unsigned long long read_perf_counter(int fd);

#endif
