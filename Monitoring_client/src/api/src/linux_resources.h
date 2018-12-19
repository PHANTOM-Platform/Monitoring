#ifndef _RESOURCES_MONITOR_H
#define _RESOURCES_MONITOR_H

#define METRIC_NAME_1 "resources_usage"

typedef struct resources_stats_t {
	unsigned long MemTotal;
	unsigned long SwapTotal;
	unsigned long pid_VmRSS, pid_VmSwap;
	float CPU_usage_rate,  max_CPU_usage_rate, min_CPU_usage_rate; long long int accum_CPU_usage_rate;
	float RAM_usage_rate,  max_RAM_usage_rate, min_RAM_usage_rate; long long int accum_RAM_usage_rate;
	float swap_usage_rate, max_swap_usage_rate, min_swap_usage_rate; long long int accum_swap_usage_rate;
	unsigned long long process_CPU_time, before_process_CPU_time;
	unsigned long long total_cpu_time, before_total_cpu_time;
	unsigned long long before_accum_write_bytes;
	unsigned long long write_bytes,  max_write_bytes, min_write_bytes, accum_write_bytes;
	unsigned long long before_accum_read_bytes;
	unsigned long long read_bytes,   max_read_bytes, min_read_bytes, accum_read_bytes;
// 	unsigned long long total_io_bytes;
	
	unsigned long long rcv_bytes, max_rcv_bytes, min_rcv_bytes , accum_rcv_bytes;
	unsigned long long send_bytes, max_send_bytes, min_send_bytes, accum_send_bytes ;
	float throughput,  max_throughput, min_throughput;
// 	unsigned long long total_net_bytes;
} resources_stats;

int linux_resources(int pid, char *DataPath, long sampling_interval);

#endif
