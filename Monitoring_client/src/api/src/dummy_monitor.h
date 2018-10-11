#ifndef _DUMMY_MONITOR_H
#define _DUMMY_MONITOR_H

#define METRIC_NAME_5 "dummy"

typedef struct dummy_stats_t {
	unsigned long long data_before;
	unsigned long long data_after;
} dummy_stats;

int dummy_monitor(int pid, char *DataPath, long sampling_interval);
int dummy_get_stats(int pid, dummy_stats *dummy_info);

#endif

