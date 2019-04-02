#ifndef _POWER_MONITOR_H
#define _POWER_MONITOR_H

#define METRIC_NAME_3 "power"
#include "mf_api.h"
#include "linux_resources.h"

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

int power_monitor(int pid, char *DataPath, long sampling_interval, long long int start_app_time, struct app_report_t *my_app_report, struct task_data_t *my_task_data_a);
#endif
