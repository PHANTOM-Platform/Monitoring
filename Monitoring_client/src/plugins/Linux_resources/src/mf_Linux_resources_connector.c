/*
* Copyright (C) 2018 University of Stuttgart
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include "mf_Linux_resources_connector.h"

#define SUCCESS 0
#define FAILURE 1

#define CPU_STAT_FILE "/proc/stat"
#define RAM_STAT_FILE "/proc/meminfo"
#define NET_STAT_FILE "/proc/net/dev"
#define IO_STAT_FILE "/proc/%d/io"

#define RESOURCES_EVENTS_NUM 5
#define HAS_CPU_STAT 0x01
#define HAS_RAM_STAT 0x02
#define HAS_SWAP_STAT 0x04
#define HAS_NET_STAT 0x08
#define HAS_IO_STAT 0x10
//the corresponding 5 string labels:
const char Linux_resources_metrics[RESOURCES_EVENTS_NUM][32] = {
	"cpu_usage_rate", "ram_usage_rate", "swap_usage_rate",
	"net_throughput", "io_throughput"};
/*******************************************************************************
* Variable Declarations
******************************************************************************/
/* flag indicates which events are given as input */
unsigned int flag = 0;
/* time in seconds */
double before_time, after_time;

struct cpu_stats {
	unsigned long long total_cpu_time;
	unsigned long long total_idle_time;
};
struct cpu_stats cpu_stat_before;
struct cpu_stats cpu_stat_after;

struct net_stats {
	unsigned long long rcv_bytes;
	unsigned long long send_bytes;
};
struct net_stats net_stat_before;
struct net_stats net_stat_after;

struct io_stats {
	unsigned long long read_bytes;
	unsigned long long write_bytes;
};
struct io_stats io_stat_before;
struct io_stats io_stat_after;

//assumes little endian
void printBits(size_t const size, void const * const ptr) {
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    for (i=size-1;i>=0;i--) {
        for (j=7;j>=0;j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
   printf("\n");
}
// 		printf(" flagz %i ");
// 	 printBits(sizeof(flag), &flag);

/*******************************************************************************
* Functions implementation
******************************************************************************/
/** @brief Adds events to the data->events, if the events are valid
 * updates the global variable flag in function of events and num_events
 @return SUCCESS or otherwise FAILURE*/
int flag_init(char **events, size_t num_events) {
	int i, ii;
// 	printf(" num_events %i RESOURCES_EVENTS_NUM %i \n",(int)num_events,RESOURCES_EVENTS_NUM);
	for (i=0; i < num_events; i++) {
		for (ii = 0; ii < RESOURCES_EVENTS_NUM; ii++) {
			/* if events name matches */
// 			printf(" %i-%i    comp (%s  % s )\n",i,ii,events[i], Linux_resources_metrics[ii]);
			if(strcmp(events[i], Linux_resources_metrics[ii]) == 0) {
				/* get the flag updated */
				unsigned int current_event_flag = 1 << ii;
				flag = flag | current_event_flag;
// 				printf(" :::> %i %i \n", flag, current_event_flag);
				break;
			}
		}
	}
	if (flag == 0) {
// 		printf("<<<< flag %i\n",flag);
		fprintf(stderr, "3-Wrong given metrics.\nPlease given metrics: ");
		for (ii = 0; ii < RESOURCES_EVENTS_NUM; ii++)
			fprintf(stderr, "%s ", Linux_resources_metrics[ii]);
		fprintf(stderr, "\n");
		return FAILURE;
	}
	printf("return success");
	return SUCCESS;
}

/** @brief Gets the current system clocks (cpu runtime, idle time, and so on)
 * collected metrics on cpu_info->total_cpu_time and cpu_info->total_idle_time
 @return SUCCESS or otherwise FAILURE*/
int CPU_stat_read(struct cpu_stats *cpu_info) {
	FILE *fp;
	char line[1024];
	unsigned long long cpu_user=0, cpu_nice=0, cpu_sys=0, cpu_idle=0, cpu_iowait=0, cpu_irq=0, cpu_softirq=0, cpu_steal=0;
	fp = fopen(CPU_STAT_FILE, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", CPU_STAT_FILE);
		return FAILURE;
	}
	if (fgets(line, 1024, fp) != NULL) {
		sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu %llu",
			&cpu_user, &cpu_nice,
			&cpu_sys, &cpu_idle,
			&cpu_iowait, &cpu_irq,
			&cpu_softirq, &cpu_steal);
	}
	cpu_info->total_cpu_time = cpu_user + cpu_nice + cpu_sys + cpu_idle + cpu_iowait + cpu_irq + cpu_softirq + cpu_steal;
	cpu_info->total_idle_time = cpu_idle + cpu_iowait;
	fclose(fp);
	return SUCCESS;
}

/** @brief Gets ram usage rate (unit is %)
 * collected metrics on MemTotal and MemFree
 @return the ram usage rate on success; 0.0 otherwise */
float RAM_usage_rate_read() {
	FILE *fp;
	char line[1024];
	int MemTotal = 0;
	int MemFree = 0;
	float RAM_usage_rate = 0.0;
	fp = fopen(RAM_STAT_FILE, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", RAM_STAT_FILE);
		return 0.0;
	}
	while(fgets(line, 1024, fp) != NULL) {
		if (!strncmp(line, "MemTotal:", 9)) {
			sscanf(line + 9, "%d", &MemTotal);
		}
		if (!strncmp(line, "MemFree:", 8)) {
			sscanf(line + 8, "%d", &MemFree);
		}
		if ((MemTotal * MemFree) != 0) {
			RAM_usage_rate = (MemTotal - MemFree) * 100.0 / MemTotal;
			break;
		}
	}
	fclose(fp);
	return RAM_usage_rate;
}

/** @brief Gets swap usage rate (unit is %)
 * collected metrics on SwapTotal and SwapFree
 @return the swap-ram usage rate on success; 0.0 otherwise */
float swap_usage_rate_read() {
	FILE *fp;
	char line[1024];
	int SwapTotal = 0;
	int SwapFree = 0;
	float swap_usage_rate = 0.0;
	fp = fopen(RAM_STAT_FILE, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", RAM_STAT_FILE);
		return 0.0;
	}
	while(fgets(line, 1024, fp) != NULL) {
		if (!strncmp(line, "SwapTotal:", 10))
			sscanf(line + 10, "%d", &SwapTotal);
		if (!strncmp(line, "SwapFree:", 9))
			sscanf(line + 9, "%d", &SwapFree);
		if ((SwapTotal * SwapFree) != 0) {
			swap_usage_rate = (SwapTotal - SwapFree) * 100.0 / SwapTotal;
			break;
		}
	}
	fclose(fp);
	return swap_usage_rate;
}

/** @brief Gets current network stats (send and receive bytes)
 * collected metrics on nets_info->rcv_bytes and nets_info->send_bytes
 @return SUCCESS or otherwise FAILURE*/
int NET_stat_read(struct net_stats *nets_info) {
	FILE *fp;
	char line[1024];
	unsigned int temp;
	unsigned long long temp_rcv_bytes, temp_send_bytes;
	fp = fopen(NET_STAT_FILE, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", NET_STAT_FILE);
		return FAILURE;
	}
	/* values reset to zeros */
	nets_info->rcv_bytes = 0;
	nets_info->send_bytes = 0;
	while(fgets(line, 1024, fp) != NULL) {
		char *sub_line_eth = strstr(line, "eth");
		if (sub_line_eth != NULL) {
			sscanf(sub_line_eth + 5, "%llu%u%u%u%u%u%u%u%llu", 
				&temp_rcv_bytes, &temp, &temp, &temp, &temp, &temp, &temp, &temp,
				&temp_send_bytes);
			nets_info->rcv_bytes += temp_rcv_bytes;
			nets_info->send_bytes += temp_send_bytes;
		}
		char *sub_line_wlan = strstr(line, "wlan");
		if (sub_line_wlan != NULL) {
			sscanf(sub_line_wlan + 6, "%llu%u%u%u%u%u%u%u%llu", 
				&temp_rcv_bytes, &temp, &temp, &temp, &temp, &temp, &temp, &temp,
				&temp_send_bytes);
			nets_info->rcv_bytes += temp_rcv_bytes;
			nets_info->send_bytes += temp_send_bytes;
		}
	}
	fclose(fp);
	return SUCCESS;
}

/** @brief Gets the IO stats of a specified process
 * collected metrics on  io_info->read_bytes and io_info->write_bytes
 @return SUCCESS or otherwise FAILURE*/
int process_IO_stat_read(int pid, struct io_stats *io_info) {
	FILE *fp;
	char filename[128], line[256];
	io_info->read_bytes = 0;
	io_info->write_bytes = 0;
	/* Gets the filename /proc/[pid]/io and open the file for reading */
	sprintf(filename, IO_STAT_FILE, pid);
	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", filename);
		return FAILURE;
	}
	/* Gets the read and write bytes for the process */
	while (fgets(line, 256, fp) != NULL) {
		if (!strncmp(line, "read_bytes:", 11)) {
			sscanf(line + 12, "%llu", &io_info->read_bytes);
		}
		if (!strncmp(line, "write_bytes:", 12)) {
			sscanf(line + 13, "%llu", &io_info->write_bytes);
		}
		if ((io_info->read_bytes * io_info->write_bytes) != 0)
			break;
	}
	fclose(fp);
	return SUCCESS;
}

/** @brief Gets the IO stats of the whole system (read IO stats for all processes and make an addition)
 * collected metrics on  total_io_stat->read_bytes and total_io_stat->write_bytes
 @return SUCCESS or otherwise FAILURE*/
int sys_IO_stat_read(struct io_stats *total_io_stat) {
	/* reset total_io_stat into zeros */
	total_io_stat->read_bytes = 0;
	total_io_stat->write_bytes = 0;
	return FAILURE; //we skip ...
	
	
	DIR *dir;
	struct dirent *drp;
	int pid;
	
	/* open /proc directory */
	dir = opendir("/proc");
	if (dir == NULL) {
		fprintf(stderr, "Error: Cannot open /proc.\n");
		return FAILURE;
	}
	/* declare data structure which stores the io stattistics of each process */
	struct io_stats pid_io_stat;

	/* get the entries in the /proc directory */
	drp = readdir(dir);
	while (drp != NULL) {
		/* if entry's name starts with digit,
		get the process pid and read the IO stats of the process */
		if (isdigit(drp->d_name[0])) {
			pid = atoi(drp->d_name);
			if (process_IO_stat_read(pid, &pid_io_stat)) {
				total_io_stat->read_bytes += pid_io_stat.read_bytes;
				total_io_stat->write_bytes += pid_io_stat.write_bytes;
			}
		}
		/* read the next entry in the /proc directory */
		drp = readdir(dir);
	}
	/* close /proc directory */
	closedir(dir);
	return SUCCESS;
}




/** @brief Initializes the Linux_resources plugin
*  Check if input events are valid; add valid events to the data->events
*  acquire the previous value and before timestamp
*  @return SUCCESS on success; FAILURE otherwise.
*/
int mf_Linux_resources_init(Plugin_metrics *data, char **events, size_t num_events) {
	/* failed to initialize flag means that all events are invalid */
	printf(" mf_Linux_resources_init  ....\n");
	if(flag_init(events, num_events) == FAILURE)
		return FAILURE;
	/* get the before timestamp in second */
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
	before_time = timestamp.tv_sec * 1.0  + (double)(timestamp.tv_nsec / 1.0e9);
	/* initialize Plugin_metrics events' names according to flag */
	int i = 0;
	if(flag & HAS_CPU_STAT) {
		data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));
		strcpy(data->events[i], "CPU_usage_rate" );// "CPU_usage_rate" Linux_resources_metrics[0]
		CPU_stat_read(&cpu_stat_before);
		i++;
	}
	if(flag & HAS_RAM_STAT) {
		data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));
		strcpy(data->events[i],"RAM_usage_rate" );// "RAM_usage_rate" Linux_resources_metrics[1]
		i++;
	}
	if(flag & HAS_SWAP_STAT) {
		data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));
		strcpy(data->events[i], Linux_resources_metrics[2]);//"swap_usage_rate"
		i++;
	}
	if(flag & HAS_NET_STAT) {
		data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));
		strcpy(data->events[i], Linux_resources_metrics[3] );//"net_throughput"
		/* read the current network rcv/send bytes */
		NET_stat_read(&net_stat_before);
		i++;
	}
	if(flag & HAS_IO_STAT) {
		data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));
		strcpy(data->events[i], Linux_resources_metrics[4] );//"io_throughput"
		/* read the current io read/write bytes for all processes */
		sys_IO_stat_read(&io_stat_before);
		i++;
	}
	data->num_events = i;
	return 1; // IMPORTANT it is expected >0 by load_plugin
}

/** @brief Samples all possible events and stores data into the Plugin_metrics
*  @return SUCCESS on success; FAILURE otherwise.
*/
int mf_Linux_resources_sample(Plugin_metrics *data) {
	/* get current timestamp in second */
	struct timespec timestamp;
	int i;
	for (i = 0; i < data->num_events; i++)
		data->values[i] = 0.0;
	clock_gettime(CLOCK_REALTIME, &timestamp);
	after_time = timestamp.tv_sec * 1.0  + (double)(timestamp.tv_nsec / 1.0e9);
	double time_interval = after_time - before_time;
	i = 0;
	if(flag & HAS_CPU_STAT) {
		CPU_stat_read(&cpu_stat_after);
		if(cpu_stat_after.total_cpu_time > cpu_stat_before.total_cpu_time) {
			data->values[i] = ((cpu_stat_after.total_cpu_time - cpu_stat_before.total_cpu_time) - (cpu_stat_after.total_idle_time - cpu_stat_before.total_idle_time)) * 100.0 /
					(cpu_stat_after.total_cpu_time - cpu_stat_before.total_cpu_time);
			/* update the cpu_stat_before values by the current values */
			cpu_stat_before.total_cpu_time = cpu_stat_after.total_cpu_time;
			cpu_stat_before.total_idle_time = cpu_stat_after.total_idle_time;
		}
		i++;
	}
	if(flag & HAS_RAM_STAT) {
		data->values[i] = RAM_usage_rate_read();
		i++;
	}
	if(flag & HAS_SWAP_STAT) {
		data->values[i] = swap_usage_rate_read();
		i++;
	}
	if(flag & HAS_NET_STAT) {
		NET_stat_read(&net_stat_after);
		unsigned long long total_bytes = (net_stat_after.rcv_bytes - net_stat_before.rcv_bytes) 
			+ (net_stat_after.send_bytes - net_stat_before.send_bytes);
		if(total_bytes > 0.0) {
			data->values[i] = (float) (total_bytes * 1.0 / time_interval);
			/* update the net_stat_before values by the current values */
			net_stat_before.rcv_bytes = net_stat_after.rcv_bytes;
			net_stat_before.send_bytes = net_stat_after.send_bytes;
		}
		i++;
	}
	if(flag & HAS_IO_STAT) {
		sys_IO_stat_read(&io_stat_after);
		unsigned long long total_bytes = (io_stat_after.read_bytes - io_stat_before.read_bytes)
			+ (io_stat_after.write_bytes - io_stat_before.write_bytes);
		if(total_bytes > 0.0) {
			data->values[i] = (float) (total_bytes * 1.0 / time_interval);
			/* update the io_stat_before values by the current values */
			io_stat_before.read_bytes = io_stat_after.read_bytes;
			io_stat_before.write_bytes = io_stat_after.write_bytes;
		}
		i++;
	}
	/* update timestamp */
	before_time = after_time;
	return SUCCESS;
}

/** @brief Formats the sampling data into a json string
*  json string contains: plugin name, timestamps, metrics_name and metrics_value
*/
void mf_Linux_resources_to_json(Plugin_metrics *data, const char *device_id, char *json) {
	char tmp[128] = {'\0'};
	int i;
	/* prepares the json string, including current timestamp, and name of the plugin */
	sprintf(json, "\"type\":\"Linux_resources\", \"host\":\"%s\"",device_id);
	sprintf(tmp, ",\"local_timestamp\":\"%.1f\"", after_time * 1.0e3);
// 	sprintf(json, "%%22type%%22:%%22Linux_resources%%22,%%22host%%22:%%22%s%%22",device_id);
// 	sprintf(tmp, ",%%22local_timestamp%%22:%%22%.1f%%22", after_time * 1.0e3);
	
	strcat(json, tmp);
	/* filters the sampled data with respect to metrics values */
	for (i = 0; i < data->num_events; i++) {
		/* if metrics' value >= 0.0, append the metrics to the json string */
		if(data->values[i] >= 0.0) {
			sprintf(tmp, ",\"%s\":%.3f", data->events[i], data->values[i]);
// 			sprintf(tmp, ",%%22%s%%22:%.3f", data->events[i], data->values[i]);			
			strcat(json, tmp);
		}
	}
// 	printf("events: %i json is %s\n",data->num_events,json);
}
