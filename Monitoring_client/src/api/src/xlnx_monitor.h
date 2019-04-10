#ifndef _XLNX_MONITOR_H
#define _XLNX_MONITOR_H

#define METRIC_NAME_2 "xlnx_monitor"

typedef struct xlnx_stats_t {
	char name[30];
	long long int temp;//temperature
	long long avg_temp;//temperature
	long long int min_temp;//temperature
	long long int max_temp;//temperature
	long long current[5], volt[5]; //value is always positive, but we wish to consider sign to detect overflow
	long long avg_current[5], avg_volt[5]; //value is always positive, but we wish to consider sign to detect overflow
	long long max_current[5], max_volt[5];
	long long min_current[5], min_volt[5];
	float total_watts, acum_total_watts;
} xlnx_stats;

int xlnx_monitor(char *DataPath, long sampling_interval);

#endif
