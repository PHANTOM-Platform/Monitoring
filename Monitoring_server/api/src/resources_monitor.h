#ifndef _RESOURCES_MONITOR_H
#define _RESOURCES_MONITOR_H

#define METRIC_NAME_1 "resources_usage"

typedef struct resources_stats_t {
	float CPU_usage_rate;
	float RAM_usage_rate;
	float swap_usage_rate;
} resources_stats;

typedef struct resources_cpu_t {
	unsigned long long process_CPU_time;
	unsigned long long global_CPU_time;
} resources_cpu;

int resources_monitor(int pid, char *DataPath, long sampling_interval);
int resources_stat_cpu(int pid, resources_cpu *stats_now);
int resources_stat_all_and_calculate(int pid, resources_cpu *before, resources_cpu *after, resources_stats *result);

#endif