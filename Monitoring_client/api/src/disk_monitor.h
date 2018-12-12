#ifndef _DISK_MONITOR_H
#define _DISK_MONITOR_H

#define METRIC_NAME_2 "disk_io"

typedef struct disk_stats_t {
	unsigned long long read_bytes_before;
	unsigned long long read_bytes_after;
	unsigned long long write_bytes_before;
	unsigned long long write_bytes_after;
} disk_stats;

int disk_monitor(int pid, char *DataPath, long sampling_interval);
int disk_stats_read(int pid, disk_stats *disk_info);

#endif

