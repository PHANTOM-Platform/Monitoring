/*
 * Copyright 2018 High Performance Computing Center, Stuttgart
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
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "linux_resources.h"
#include "mf_api.h"
#include "mf_util.h"
// #include "util.h"

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
const char Linux_resources_metrics[RESOURCES_EVENTS_NUM][32] = {
	"cpu_usage_rate", "ram_usage_rate", "swap_usage_rate",
	"net_throughput", "io_throughput"};
/******************************************************************************
* Variable Declarations
******************************************************************************/
/* flag indicates which events are given as input */
unsigned int flag = 0;
/* time in seconds */
// double before_time, after_time;
/*******************************************************************************
 * Implementaion
 ******************************************************************************/
 
 /** @brief updates the global variable flag in function of events and num_events
 @return SUCCESS or otherwise FAILURE*/
int flag_init(char **events, size_t num_events) {
	int i, ii;
	for (i=0; i < num_events; i++) {
		for (ii = 0; ii < RESOURCES_EVENTS_NUM; ii++) {
			/* if events name matches */
			if(strcmp(events[i], Linux_resources_metrics[ii]) == 0) {
				/* get the flag updated */
				unsigned int current_event_flag = 1 << ii;
				flag = flag | current_event_flag;
			}
		}
	}
	if (flag == 0) {
		fprintf(stderr, "Wrong given metrics.\nPlease given metrics ");
		for (ii = 0; ii < RESOURCES_EVENTS_NUM; ii++)
			fprintf(stderr, "%s ", Linux_resources_metrics[ii]);
		fprintf(stderr, "\n");
		return FAILURE;
	}
	return SUCCESS;
}


/** @brief Gets the current system clocks (cpu runtime, idle time, and so on)
 * collected metrics on stats_now->total_cpu_time
 @return SUCCESS or otherwise FAILURE*/
int CPU_stat_read(struct resources_stats_t *stats_now){
	FILE *fp;
	char line[1024];
	unsigned long long tmp;
	unsigned long long cpu_user=0, cpu_sys=0;
	stats_now->total_cpu_time=0;
	fp = fopen(CPU_STAT_FILE, "r");
	if(fp == NULL) {
		printf("ERROR: Could not open file %s\n", CPU_STAT_FILE);
		exit(0);
	}
	if(fgets(line, 1024, fp) != NULL) {
		sscanf(line+5, "%llu %llu %llu %llu %llu %llu %llu %llu",
			&cpu_user, &tmp, &cpu_sys, &tmp, &tmp, &tmp, &tmp, &tmp);
	}
	stats_now->before_total_cpu_time=stats_now->total_cpu_time;
	stats_now->total_cpu_time = cpu_user + cpu_sys;
	fclose(fp);
	return SUCCESS;
}

/*read cpu user time and system time from /proc/[pid]/stat */
/*read cpu user time and system time from CPU_STAT_FILE=/proc/stat */
int CPU_stat_process_read(int pid, struct resources_stats_t *stats_now) {
	FILE *fp;
	char line[1024];
	unsigned long long tmp;
	char pid_cpu_file[128] = {'\0'};
	unsigned long long pid_utime, pid_stime;
	sprintf(pid_cpu_file, "/proc/%d/stat", pid);
	fp = fopen(pid_cpu_file, "r");
	stats_now->process_CPU_time = 0;
	if(fp == NULL) {
		printf("ERROR: Could not open file %s\n", pid_cpu_file);
		exit(0);
	}
	char tmp_str[32];
	char tmp_char;
	if(fgets(line, 1024, fp) != NULL) {
		sscanf(line, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu",
			(int *)&tmp, tmp_str, &tmp_char, (int *)&tmp, (int *)&tmp, (int *)&tmp, (int *)&tmp, (int *)&tmp,
			(unsigned int *)&tmp, (unsigned long *)&tmp, (unsigned long *)&tmp, (unsigned long *)&tmp, 
			(unsigned long *)&tmp, (unsigned long *)&pid_utime, (unsigned long *)&pid_stime);
	}
	stats_now->before_process_CPU_time = stats_now->process_CPU_time;
	stats_now->process_CPU_time = pid_utime + pid_stime;
	fclose(fp);
	return SUCCESS;
}


/** @brief Gets ram usage rate (unit is %) from RAM_STAT_FILE=/proc/meminfo
 * collected metrics on MemTotal and MemFree
 @return e */
int RAM_usage_rate_read(struct resources_stats_t *stats_now) {
	FILE *fp;
	char line[1024];
// 	unsigned long MemFree = 0;
// 	float RAM_usage_rate = 0.0;
	fp = fopen(RAM_STAT_FILE, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", RAM_STAT_FILE);
		return FAILURE;
	}
	while(fgets(line, 1024, fp) != NULL) {
		if (!strncmp(line, "MemTotal:", 9)) {
			sscanf(line + 9, "%lu", &stats_now->MemTotal);
		}
		if (!strncmp(line, "SwapTotal:", 10)) {
			sscanf(line + 10, "%lu", &stats_now->SwapTotal);
		}
// 		if (!strncmp(line, "MemFree:", 8)) {
// 			sscanf(line + 8, "%d", &MemFree);
// 		}
// 		if ((MemTotal * MemFree) != 0) {
// 			RAM_usage_rate = (MemTotal - MemFree) * 100.0 / MemTotal;
// 			break;
// 		}
	}
	fclose(fp);
	return SUCCESS;
}


/** @brief read VmRSS and VmSwap from /proc/[pid]/status
 * collected metrics on MemTotal and MemFree
 @return e */
int RAM_process_usage_rate_read(const int pid, struct resources_stats_t *stats_now) {
	FILE *fp;
	char line[1024];
	char pid_status_file[128] = {'\0'};
	sprintf(pid_status_file, "/proc/%d/status", pid);
	fp = fopen(pid_status_file, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", pid_status_file);
		return FAILURE;
	}
	while(fgets(line, 1024, fp) != NULL) {
		if (!strncmp(line, "VmRSS:", 6)) {
			sscanf(line + 6, "%lu", &stats_now->pid_VmRSS);
		}
		if (!strncmp(line, "VmSwap:", 7)) {
			sscanf(line + 7, "%lu", &stats_now->pid_VmSwap);
		}
	}
	fclose(fp);
	return SUCCESS;
}

int io_stats_read(int pid, struct resources_stats_t *stats_now) {
	FILE *fp;
	stats_now->accum_read_bytes=0;
	stats_now->accum_write_bytes=0;
	char pid_io_file[128], line[1024];
	sprintf(pid_io_file, "/proc/%d/io", pid);
	if ((fp = fopen(pid_io_file, "r")) == NULL) {
		fprintf(stderr, "ERROR: Could not open file %s.\n", pid_io_file);
		return FAILURE;
	}
	while (fgets(line, 256, fp) != NULL) {
		if (!strncmp(line, "read_bytes:", 11))
			sscanf(line + 12, "%llu", &stats_now->accum_read_bytes);
		if (!strncmp(line, "write_bytes:", 12))
			sscanf(line + 13, "%llu", &stats_now->accum_write_bytes);
	}
	fclose(fp);
	return SUCCESS;
}


/** @brief Gets current network stats (send and receive bytes)
 * collected metrics on nets_info->rcv_bytes and nets_info->send_bytes
 @return SUCCESS or otherwise FAILURE*/
int network_stat_read(int pid, struct resources_stats_t *nets_info) {
	FILE *fp;
	unsigned int temp;
	unsigned long long temp_rcv_bytes, temp_send_bytes;
	/* values reset to zeros */
	nets_info->rcv_bytes = 0;
	nets_info->send_bytes = 0;
	char pid_net_file[128], line[1024];
	sprintf(pid_net_file, "/proc/%d/net/dev", pid);//NET_STAT_FILE
	if ((fp = fopen(pid_net_file, "r")) == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", pid_net_file);
		return FAILURE;
	}
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











/** @brief Initializes the Linux_resources plugin
* acquire the previous value and before timestamp
* @return SUCCESS on success; FAILURE otherwise.
*/
int linux_resources_init(int pid, char **events, size_t num_events, struct resources_stats_t stats_now) {
	/* failed to initialize flag means that all events are invalid */
	if(flag_init(events, num_events) == 0)
		return FAILURE;
	/* get the before timestamp in second */
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
// 	before_time = timestamp.tv_sec * 1.0 + (double)(timestamp.tv_nsec / 1.0e9);
	/* initialize Plugin_metrics events' names according to flag */
	int i = 0;
	if(flag & HAS_CPU_STAT) {
		CPU_stat_read(&stats_now);
		CPU_stat_process_read(pid, &stats_now);
		i++;
	}
	if(flag & HAS_RAM_STAT) {
		//
		
	}
	if(flag & HAS_SWAP_STAT) {
		//ggg
	}
	if(flag & HAS_NET_STAT) {
// 		/* read the current network rcv/send bytes */
		network_stat_read(pid, &stats_now);
	}
	if((flag & HAS_RAM_STAT)|| (flag & HAS_SWAP_STAT)) {
		RAM_process_usage_rate_read(pid, &stats_now);
		RAM_usage_rate_read( &stats_now);
		/*calculate for the resources_stats */
		stats_now.RAM_usage_rate = (stats_now.MemTotal==0) ? 0 : stats_now.pid_VmRSS * 100.0 /stats_now.MemTotal;
		stats_now.swap_usage_rate = (stats_now.SwapTotal==0) ? 0 : stats_now.pid_VmSwap * 100.0 /stats_now.SwapTotal;
	}
	if(flag & HAS_IO_STAT) {
// 		/* read the current io read/write bytes for all processes */
		/*initialize the values in result */
		stats_now.before_accum_read_bytes = stats_now.accum_read_bytes;
		stats_now.before_accum_write_bytes = stats_now.accum_write_bytes;
		io_stats_read(pid, &stats_now);
		i++;
	}
	return SUCCESS;
}

/** @brief Samples all possible events and stores data into the Plugin_metrics
* @return SUCCESS on success; FAILURE otherwise.
*/
int linux_resources_sample(const int pid, struct resources_stats_t stat_after ) {
	/* get current timestamp in second */
	struct timespec timestamp;
	int i;
	clock_gettime(CLOCK_REALTIME, &timestamp);
// 	after_time = timestamp.tv_sec * 1.0 + (double)(timestamp.tv_nsec / 1.0e9);
// 	double time_interval = after_time - before_time;
	i = 0;
	if(flag & HAS_CPU_STAT) {
		CPU_stat_read(&stat_after);
		CPU_stat_process_read(pid, &stat_after);
		if(stat_after.total_cpu_time > stat_after.before_total_cpu_time) {
// 			data->values[i] = ((stat_after.total_cpu_time - stat_after.before_total_cpu_time) - (stat_after.total_idle_time - stat_before.total_idle_time)) * 100.0 /
// 					(stat_after.total_cpu_time - stat_after.before_total_cpu_time);
			/* update the stat_before values by the current values */
			stat_after.before_total_cpu_time = stat_after.total_cpu_time;
// 			stat_before.total_idle_time = stat_after.total_idle_time;
		}
		if((stat_after.process_CPU_time <= stat_after.before_process_CPU_time)
			|| (stat_after.total_cpu_time <= stat_after.before_total_cpu_time)) {
			stat_after.CPU_usage_rate = 0.0;
		} else {
			stat_after.CPU_usage_rate = (stat_after.process_CPU_time - stat_after.before_process_CPU_time) * 100.0 / 
				(stat_after.total_cpu_time - stat_after.before_total_cpu_time);
		}
		i++;
	}
	if((flag & HAS_RAM_STAT)|| (flag & HAS_SWAP_STAT)) {
		RAM_process_usage_rate_read(pid, &stat_after);
		RAM_usage_rate_read( &stat_after);
		/*calculate for the resources_stats */
		stat_after.RAM_usage_rate = (stat_after.MemTotal==0) ? 0 : stat_after.pid_VmRSS * 100.0 /stat_after.MemTotal;
		stat_after.swap_usage_rate = (stat_after.SwapTotal==0) ? 0 : stat_after.pid_VmSwap * 100.0 /stat_after.SwapTotal;
		i++;
	}
	if(flag & HAS_NET_STAT) {
		network_stat_read(pid, &stat_after);
		unsigned long long total_bytes = (stat_after.rcv_bytes - stat_after.rcv_bytes) 
			+ (stat_after.send_bytes - stat_after.send_bytes);
		if(total_bytes > 0.0) {
			/* update the stat_after values by the current values */
			stat_after.rcv_bytes = stat_after.rcv_bytes;
			stat_after.send_bytes = stat_after.send_bytes;
		}
		i++;
	}
	if(flag & HAS_IO_STAT) {
		io_stats_read(pid, &stat_after);
// 		stat_after.total_io_bytes = (stat_after.accum_read_bytes - stat_after.before_accum_read_bytes)
// 			+ (stat_after.accum_write_bytes - stat_after.before_accum_write_bytes);
// 		if(total_bytes > 0.0) {
// 			data->values[i] = (float) (total_bytes * 1000.0 / time_interval);
// 		}
		i++;
	}
	/* update timestamp */
// 	before_time = after_time;
	return SUCCESS;
}

struct resources_stats_t *stats=NULL;



char *save_stats_resources( struct resources_stats_t *stat, int pretty, int tabs){
	int i;
	char tempstr[2048]={'\0'};
	char *msg=NULL;
// 	----------------------CPU
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_cpu_usage_rate" );concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%.2f\"", stats->min_CPU_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%.2f\"", stats->max_CPU_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%.2f\"", (float) stats->accum_CPU_usage_rate/ (float)stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"},", stats->accum_CPU_usage_rate);concat_and_free(&msg, tempstr);
// 	----------------------RAM
	if (pretty==1) concat_and_free(&msg, "\n");
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_ram_usage_rate");concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%.2f\"", stats->min_RAM_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%.2f\"", stats->max_RAM_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%.2f\"", (float)stats->accum_RAM_usage_rate/(float)stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"},", stats->accum_RAM_usage_rate);concat_and_free(&msg, tempstr);
// 	----------------------SWAP
	if (pretty==1) concat_and_free(&msg, "\n");
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_swap_usage_rate");concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%.2f\"", stats->min_swap_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%.2f\"", stats->max_swap_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%.2f\"", (float)stats->accum_swap_usage_rate/(float)stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"},", stats->accum_swap_usage_rate);concat_and_free(&msg, tempstr);
// 	---------------------- IO
	if (pretty==1) concat_and_free(&msg, "\n");
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_write_bytes");concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%lli\"", stats->min_write_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%lli\"", stats->max_write_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%lli\"", stats->accum_write_bytes/stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"},", stats->accum_write_bytes);concat_and_free(&msg, tempstr);
// ------------------------ IO
	if (pretty==1) concat_and_free(&msg, "\n");
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_read_bytes");concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%lli\"", stats->min_read_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%lli\"", stats->max_read_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%lli\"", stats->accum_read_bytes/stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"},", stats->accum_read_bytes);concat_and_free(&msg, tempstr);
// -------------------------NET
	if (pretty==1) concat_and_free(&msg, "\n");
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_send_bytes");concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%lli\"", stats->min_send_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%lli\"", stats->max_send_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%lli\"", stats->accum_send_bytes/stats->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"}", stats->accum_send_bytes);concat_and_free(&msg, tempstr);
//		nets_info->rcv_bytes += temp_rcv_bytes;
//		nets_info->send_bytes += temp_send_bytes;
	if(pretty==0) concat_and_free(&msg,  "\n");
	return msg;
}

char *save_stats_resources_comp( struct  sub_task_data *subtask, int pretty, int tabs){
	int i;
	char tempstr[2048]={'\0'};
	char *msg=NULL;
// 	----------------------CPU
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_cpu_usage_rate" );concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%.2f\"", subtask->min_CPU_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%.2f\"", subtask->max_CPU_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%.2f\"", (float) subtask->accum_CPU_usage_rate/ (float)subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"},", subtask->accum_CPU_usage_rate);concat_and_free(&msg, tempstr);
// 	----------------------RAM
	if (pretty==1) concat_and_free(&msg, "\n");
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_ram_usage_rate");concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%.2f\"", subtask->min_RAM_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%.2f\"", subtask->max_RAM_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%.2f\"", (float)subtask->accum_RAM_usage_rate/(float)subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"},", subtask->accum_RAM_usage_rate);concat_and_free(&msg, tempstr);
// 	----------------------SWAP
	if (pretty==1) concat_and_free(&msg, "\n");
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_swap_usage_rate");concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%.2f\"", subtask->min_swap_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%.2f\"", subtask->max_swap_usage_rate);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%.2f\"", (float)subtask->accum_swap_usage_rate/(float)subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"},", subtask->accum_swap_usage_rate);concat_and_free(&msg, tempstr);
// 	---------------------- IO
	if (pretty==1) concat_and_free(&msg, "\n");
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_write_bytes");concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%lli\"", subtask->min_write_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%lli\"", subtask->max_write_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%lli\"", subtask->accum_write_bytes/subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"},", subtask->accum_write_bytes);concat_and_free(&msg, tempstr);
// ------------------------ IO
	if (pretty==1) concat_and_free(&msg, "\n");
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_read_bytes");concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%lli\"", subtask->min_read_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%lli\"", subtask->max_read_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%lli\"", subtask->accum_read_bytes/subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"},", subtask->accum_read_bytes);concat_and_free(&msg, tempstr);
// -------------------------NET
	if (pretty==1) concat_and_free(&msg, "\n");
	if(pretty==1) for(i=0;i<tabs;i++) concat_and_free(&msg, "\t");
	sprintf(tempstr, "\"%s\":{", "stats_send_bytes");concat_and_free(&msg, tempstr);
	sprintf(tempstr, "\"count\":\"%li\"", subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"min\":\"%lli\"", subtask->min_send_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"max\":\"%lli\"", subtask->max_send_bytes);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"avg\":\"%lli\"", subtask->accum_send_bytes/subtask->counter);concat_and_free(&msg, tempstr);
	sprintf(tempstr, ",\"sum\":\"%lli\"}", subtask->accum_send_bytes);concat_and_free(&msg, tempstr);
//		nets_info->rcv_bytes += temp_rcv_bytes;
//		nets_info->send_bytes += temp_send_bytes;
	if(pretty==0) concat_and_free(&msg,  "\n");
	return msg;
}

/*
int printf_stats_resources(FILE *fp, struct resources_stats_t *stat, int pretty, int tabs){
	int i;
	if (fp==NULL || stats ==NULL) return 0;
// 	----------------------CPU
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_cpu_usage_rate" );
	fprintf(fp, "\"count\":\"%li\"", stats->counter);
	fprintf(fp, ",\"min\":\"%.2f\"", stats->min_CPU_usage_rate);
	fprintf(fp, ",\"max\":\"%.2f\"", stats->max_CPU_usage_rate);
	fprintf(fp, ",\"avg\":\"%.2f\"", (float) stats->accum_CPU_usage_rate/ (float)stats->counter);
	fprintf(fp, ",\"sum\":\"%lli\"},", stats->accum_CPU_usage_rate);
// 	----------------------RAM
	if (pretty==1) fprintf(fp,"\n");
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_ram_usage_rate");
	fprintf(fp, "\"count\":\"%li\"", stats->counter);
	fprintf(fp, ",\"min\":\"%.2f\"", stats->min_RAM_usage_rate);
	fprintf(fp, ",\"max\":\"%.2f\"", stats->max_RAM_usage_rate);
	fprintf(fp, ",\"avg\":\"%.2f\"", (float)stats->accum_RAM_usage_rate/(float)stats->counter);
	fprintf(fp, ",\"sum\":\"%lli\"},", stats->accum_RAM_usage_rate);
// 	----------------------SWAP
	if (pretty==1) fprintf(fp,"\n");
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_swap_usage_rate");
	fprintf(fp, "\"count\":\"%li\"", stats->counter);
	fprintf(fp, ",\"min\":\"%.2f\"", stats->min_swap_usage_rate);
	fprintf(fp, ",\"max\":\"%.2f\"", stats->max_swap_usage_rate);
	fprintf(fp, ",\"avg\":\"%.2f\"", (float)stats->accum_swap_usage_rate/(float)stats->counter);
	fprintf(fp, ",\"sum\":\"%lli\"},", stats->accum_swap_usage_rate);
// 	---------------------- IO
	if (pretty==1) fprintf(fp,"\n");
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_write_bytes");
	fprintf(fp, "\"count\":\"%li\"", stats->counter);
	fprintf(fp, ",\"min\":\"%lli\"", stats->min_write_bytes);
	fprintf(fp, ",\"max\":\"%lli\"", stats->max_write_bytes);
	fprintf(fp, ",\"avg\":\"%lli\"", stats->accum_write_bytes/stats->counter);
	fprintf(fp, ",\"sum\":\"%lli\"},", stats->accum_write_bytes);
// ------------------------ IO
	if (pretty==1) fprintf(fp,"\n");
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_read_bytes");
	fprintf(fp, "\"count\":\"%li\"", stats->counter);
	fprintf(fp, ",\"min\":\"%lli\"", stats->min_read_bytes);
	fprintf(fp, ",\"max\":\"%lli\"", stats->max_read_bytes);
	fprintf(fp, ",\"avg\":\"%lli\"", stats->accum_read_bytes/stats->counter);
	fprintf(fp, ",\"sum\":\"%lli\"},", stats->accum_read_bytes);
// -------------------------NET
	if (pretty==1) fprintf(fp,"\n");
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_send_bytes");
	fprintf(fp, "\"count\":\"%li\"", stats->counter);
	fprintf(fp, ",\"min\":\"%lli\"", stats->min_send_bytes);
	fprintf(fp, ",\"max\":\"%lli\"", stats->max_send_bytes);
	fprintf(fp, ",\"avg\":\"%lli\"", stats->accum_send_bytes/stats->counter);
	fprintf(fp, ",\"sum\":\"%lli\"}", stats->accum_send_bytes);
//		nets_info->rcv_bytes += temp_rcv_bytes;
//		nets_info->send_bytes += temp_send_bytes;
	if(pretty==0) fprintf(fp, "\n");
	return 0;
}*/
/*
int printf_stats_resources_comp(FILE *fp, struct  sub_task_data *subtask, int pretty, int tabs){
	int i;
	if (fp==NULL || stats ==NULL) return 0;
// 	----------------------CPU
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_cpu_usage_rate" );
	fprintf(fp, "\"count\":\"%li\"", subtask->counter);
	fprintf(fp, ",\"min\":\"%.2f\"", subtask->min_CPU_usage_rate);
	fprintf(fp, ",\"max\":\"%.2f\"", subtask->max_CPU_usage_rate);
	fprintf(fp, ",\"avg\":\"%.2f\"", (float) subtask->accum_CPU_usage_rate/ (float)subtask->counter);
	fprintf(fp, ",\"sum\":\"%lli\"},", subtask->accum_CPU_usage_rate);
// 	----------------------RAM
	if (pretty==1) fprintf(fp,"\n");
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_ram_usage_rate");
	fprintf(fp, "\"count\":\"%li\"", subtask->counter);
	fprintf(fp, ",\"min\":\"%.2f\"", subtask->min_RAM_usage_rate);
	fprintf(fp, ",\"max\":\"%.2f\"", subtask->max_RAM_usage_rate);
	fprintf(fp, ",\"avg\":\"%.2f\"", (float)subtask->accum_RAM_usage_rate/(float)subtask->counter);
	fprintf(fp, ",\"sum\":\"%lli\"},", subtask->accum_RAM_usage_rate);
// 	----------------------SWAP
	if (pretty==1) fprintf(fp,"\n");
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_swap_usage_rate");
	fprintf(fp, "\"count\":\"%li\"", subtask->counter);
	fprintf(fp, ",\"min\":\"%.2f\"", subtask->min_swap_usage_rate);
	fprintf(fp, ",\"max\":\"%.2f\"", subtask->max_swap_usage_rate);
	fprintf(fp, ",\"avg\":\"%.2f\"", (float)subtask->accum_swap_usage_rate/(float)subtask->counter);
	fprintf(fp, ",\"sum\":\"%lli\"},", subtask->accum_swap_usage_rate);
// 	---------------------- IO
	if (pretty==1) fprintf(fp,"\n");
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_write_bytes");
	fprintf(fp, "\"count\":\"%li\"", subtask->counter);
	fprintf(fp, ",\"min\":\"%lli\"", subtask->min_write_bytes);
	fprintf(fp, ",\"max\":\"%lli\"", subtask->max_write_bytes);
	fprintf(fp, ",\"avg\":\"%lli\"", subtask->accum_write_bytes/subtask->counter);
	fprintf(fp, ",\"sum\":\"%lli\"},", subtask->accum_write_bytes);
// ------------------------ IO
	if (pretty==1) fprintf(fp,"\n");
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_read_bytes");
	fprintf(fp, "\"count\":\"%li\"", subtask->counter);
	fprintf(fp, ",\"min\":\"%lli\"", subtask->min_read_bytes);
	fprintf(fp, ",\"max\":\"%lli\"", subtask->max_read_bytes);
	fprintf(fp, ",\"avg\":\"%lli\"", subtask->accum_read_bytes/subtask->counter);
	fprintf(fp, ",\"sum\":\"%lli\"},", subtask->accum_read_bytes);
// -------------------------NET
	if (pretty==1) fprintf(fp,"\n");
	if(pretty==1) for(i=0;i<tabs;i++) fprintf(fp,"\t");
	fprintf(fp, "\"%s\":{", "stats_send_bytes");
	fprintf(fp, "\"count\":\"%li\"", subtask->counter);
	fprintf(fp, ",\"min\":\"%lli\"", subtask->min_send_bytes);
	fprintf(fp, ",\"max\":\"%lli\"", subtask->max_send_bytes);
	fprintf(fp, ",\"avg\":\"%lli\"", subtask->accum_send_bytes/subtask->counter);
	fprintf(fp, ",\"sum\":\"%lli\"}", subtask->accum_send_bytes);
//		nets_info->rcv_bytes += temp_rcv_bytes;
//		nets_info->send_bytes += temp_send_bytes;
	if(pretty==0) fprintf(fp, "\n");
	return 0;
}*/

struct resources_stats_t *linux_resources(const int pid, char *DataPath, long sampling_interval) {
	char FileName[256];
	int i;
	struct timespec timestamp;
	double timestamp_ms;
	int num_events = RESOURCES_EVENTS_NUM;
	stats =(struct resources_stats_t *) malloc((num_events)* sizeof(struct resources_stats_t ));
	stats->accum_read_bytes =0;
	stats->accum_write_bytes =0;
	stats->before_accum_read_bytes = stats->accum_read_bytes;
	stats->before_accum_write_bytes = stats->accum_write_bytes;
	stats->counter=1;
	char **events;
	events= (char **) malloc((num_events)* sizeof(char *));
	for(i=0;i<num_events;i++)
		events[i]=(char*) malloc(32* sizeof(char));
	for(i=0;i<num_events;i++)
		strcpy(events[i],Linux_resources_metrics[i]);
	/*create and open the file*/
	sprintf(FileName, "%s/%s", DataPath, METRIC_NAME_1);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		exit(0);
	}
	stats->CPU_usage_rate = 0.0;
	stats->RAM_usage_rate = 0.0;
	stats->swap_usage_rate = 0.0;
	stats->send_bytes=0;
	stats->rcv_bytes=0;
	linux_resources_init(pid, events, num_events, *stats);
// 	for the case of being uninitialized
	stats->before_total_cpu_time=stats->total_cpu_time;
	stats->before_process_CPU_time=stats->process_CPU_time;

	stats->min_CPU_usage_rate = stats->CPU_usage_rate;
	stats->max_CPU_usage_rate = stats->CPU_usage_rate;

	stats->min_RAM_usage_rate = stats->RAM_usage_rate;
	stats->max_RAM_usage_rate = stats->RAM_usage_rate;

	stats->min_swap_usage_rate = stats->swap_usage_rate;
	stats->max_swap_usage_rate = stats->swap_usage_rate;

	stats->accum_CPU_usage_rate = stats->CPU_usage_rate;
	stats->accum_RAM_usage_rate = stats->RAM_usage_rate;
	stats->accum_swap_usage_rate = stats->swap_usage_rate;
	
	stats->min_write_bytes = stats->accum_write_bytes;
	stats->max_write_bytes = stats->accum_write_bytes;

	stats->min_read_bytes = stats->accum_read_bytes;
	stats->max_read_bytes = stats->accum_read_bytes;

	stats->min_read_bytes = stats->accum_read_bytes;
	stats->max_read_bytes = stats->accum_read_bytes;
	
	stats->min_send_bytes = stats->send_bytes;
	stats->max_send_bytes = stats->send_bytes;
	stats->accum_send_bytes = stats->send_bytes;
	
	/*in a loop do data sampling and write into the file*/
	while(running) {
		usleep(sampling_interval * 1000);
		linux_resources_sample(pid, *stats);

		/*get current timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp);
		timestamp_ms = timestamp.tv_sec * 1000.0 + (double)(timestamp.tv_nsec / 1.0e6);
		fprintf(fp, "\"local_timestamp\":\"%.1f\"", timestamp_ms);
		if(flag & HAS_CPU_STAT) {
			fprintf(fp, ", \"%s\":%.3f", "cpu_usage_rate", stats->CPU_usage_rate);
		}
		if(flag & HAS_RAM_STAT) {
			fprintf(fp, ", \"%s\":%.3f", "ram_usage_rate", stats->RAM_usage_rate);
		}
		if(flag & HAS_SWAP_STAT) {
			fprintf(fp, ", \"%s\":%.3f", "swap_usage_rate", stats->swap_usage_rate);
		}
		if(flag & HAS_IO_STAT) {
			/*calculate the values for disk stats */
			stats->read_bytes = stats->accum_read_bytes - stats->before_accum_read_bytes;
			stats->write_bytes = stats->accum_write_bytes - stats->before_accum_write_bytes;
			stats->throughput = (stats->read_bytes + stats->write_bytes) * 1000.0 / sampling_interval; //in bytes/s
			if(stats->min_write_bytes > stats->write_bytes) stats->min_write_bytes = stats->write_bytes;
			if(stats->max_write_bytes < stats->write_bytes) stats->max_write_bytes = stats->write_bytes;
			if(stats->min_read_bytes > stats->read_bytes) stats->min_read_bytes = stats->read_bytes;
			if(stats->max_read_bytes < stats->read_bytes) stats->max_read_bytes = stats->read_bytes;
			fprintf(fp, ", \"%s\":%llu","disk_read", stats->read_bytes);
			fprintf(fp,", \"%s\":%llu", "disk_write", stats->write_bytes);
			fprintf(fp,", \"%s\":%.3f","disk_throughput", stats->throughput);
		}
		if(flag & HAS_NET_STAT) {
			fprintf(fp, ", \"%s\":%llu", "send_bytes", stats->send_bytes);
			fprintf(fp, ", \"%s\":%llu", "rcv_bytes", stats->rcv_bytes);
			if(stats->min_send_bytes > stats->send_bytes) stats->min_send_bytes = stats->send_bytes;
			if(stats->max_send_bytes < stats->send_bytes) stats->max_send_bytes = stats->send_bytes;
			stats->accum_send_bytes += stats->send_bytes;
		}
		if(stats->min_CPU_usage_rate > stats->CPU_usage_rate) stats->min_CPU_usage_rate = stats->CPU_usage_rate;
		if(stats->max_CPU_usage_rate < stats->CPU_usage_rate) stats->max_CPU_usage_rate = stats->CPU_usage_rate;
		if(stats->min_RAM_usage_rate > stats->RAM_usage_rate) stats->min_RAM_usage_rate = stats->RAM_usage_rate;
		if(stats->max_RAM_usage_rate < stats->RAM_usage_rate) stats->max_RAM_usage_rate = stats->RAM_usage_rate;
		if(stats->min_swap_usage_rate > stats->swap_usage_rate) stats->min_swap_usage_rate = stats->swap_usage_rate;
		if(stats->max_swap_usage_rate < stats->swap_usage_rate) stats->max_swap_usage_rate = stats->swap_usage_rate;
		stats->accum_CPU_usage_rate += stats->CPU_usage_rate;
		stats->accum_RAM_usage_rate += stats->RAM_usage_rate;
		stats->accum_swap_usage_rate += stats->swap_usage_rate;
		stats->counter=stats->counter+1;
		fprintf(fp, "\n");
	}
	fclose(fp);
	sprintf(FileName, "%s/stats_%s", DataPath, METRIC_NAME_1);
	fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		return NULL;
	}
	fprintf(fp, "\"local_timestamp\":\"%.1f\",", timestamp_ms);
	
	char *msgb=save_stats_resources( stats,0,2);
// 	concat_and_free(&msg, msgb);
	fprintf(fp,"%s",msgb); free(msgb);
// 	printf_stats_resources(fp, stats,0,2);
	fclose(fp);
// 	free(stats);
	for(i=0;i<num_events;i++)
		free(events[i]);
	free(events);
	return stats;
}


//********************** EXTENSION TO MEASURE LOAD OF THREADS !!!

//author: J.M.Montanana
//Copyright HRLS 2017
//based on the use of ps and /proc/stat , /proc/diskstats, /proc/uptime, /proc/meminfo, /proc/[pid]/io, nproc

#include <stdbool.h>
#include <termios.h>
#include <unistd.h> //for sleep and usleep
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

#define MAX 200

//search for the string cad1 in the string source, starting from position "start"
//if found: return ths position where starts the cad1 in the string source
//if not found: returns a not valid value, which is the lenght of source +1
unsigned int find_str(int start, const char source[], const char cad1[]){
	int longsource=strlen(source);
	int longcad=strlen(cad1);
	if (longsource<longcad)
		return(longsource+1);
	int i=start;
	int iguales;
	do{
		iguales=true;
		for (int j=0;j<longcad;j++)
			if (source[i+j]!=cad1[j])
				iguales=false;
		i++;
	} while ((iguales==false) && (i<=(longsource-longcad)));
	if (iguales==false)
		return(longsource+1);
	i--;// Habiamos sumado uno de mas
	return (i);
}

//removes the string "cadenaBuscar" from the string "source"
//returns true if the string "cadenaBuscar" was found and removed
int remove_str(int start, char source[], const char cadenaBuscar[]){ 
	int longsource=strlen(source);
	int longcad=strlen(cadenaBuscar);
	if (longsource>=longcad){
		int i=find_str(start,source, cadenaBuscar);
		if (i+longcad<=longsource) { // Encontramos la cadena 1 y empieza en i1
			for (int j=i;j<=longsource;j++) 
				source[j]=source[j+longcad];
			return(true);
		//}else{
		//	printf(" i %i longcad %i long source %i\n",i,longcad,longsource);
		}
	}
	return(false);
}

/*
* copy from position "start" of the string "input" into the string "output"
* until find end of string, end of line or a space character ' '
*/
unsigned int process_str(char *input, char *output, unsigned const int start){
	unsigned int j=0;
	while((input[j+start]!='\n')&&(input[j+start]!='\0')&&(input[j+start]!=' ')){
		output[j]=input[j+start];
		j++;
	}
	output[j]='\0';
	return (start+j);
}


/*
* copy from position "start" of the string "input" into the string "output"
* until find end of string or end of line 
*/
unsigned int getline_str(char *input, char *output, unsigned const int start){
	unsigned int j=0;
	while((input[j+start]!='\n')&&(input[j+start]!='\0')){
		output[j]=input[j+start];
		j++;
	}
	output[j]='\0';
	return (start+j+1);
}


int find_llint_from_label(char *loadstr, const char *label, long long int *to_update){
	char result[200];
	if(remove_str(0, loadstr, label)==true){
		int i=0;
		while( (i<strlen(loadstr)) && ( ((loadstr[i]>47)&&(loadstr[i]<58)) || (loadstr[i]==' ') ) ){
			result[i]=loadstr[i];
			i++;
		}
		result[i]='\0';
		*to_update=atoll(result);
		return true;
	}else{
		return false;
	}
}


// void print_memorysize(long long int value ){
const char * str_memorysize(long long int value, char *szBuffer) {
	if(value> 1024*1024*1024){
		sprintf(szBuffer,"%5lliGB", value/(1024*1024*1024));
	}else if(value> 1024*1024){
		sprintf(szBuffer,"%5lliMB", value/(1024*1024));
	}else if(value> 1024){
		sprintf(szBuffer,"%5lliKB", value/1024);
	}else {
		sprintf(szBuffer,"%5lli B", value);
	}
	return szBuffer;
}



unsigned int print_stats(int searchprocess, task_data *my_task_data){
	printf(" %sPID",LIGHT_BLUE);
	printf("\t%sTID",LIGHT_GREEN);
	printf("\t%sCORE",RED);
	printf("\t%s%%cpu    %s%%mem",RED,LIGHT_CYAN);
	printf("%s Rchar:  ",yellow);
	printf("Wchar:  ");
	printf("Syscr:  ");
	printf("Syscw:  ");
	printf(" Read:  ");
	printf("Write:  ");
	printf("Cancel wr\n" );
	char *szBuffer=(char *)malloc(255*sizeof(char));
	for(unsigned int i=0;i<my_task_data->totaltid;i++){
		if((searchprocess == my_task_data->subtask[i]->pspid )|| (searchprocess == my_task_data->subtask[i]->pstid)){
			printf("%s%6i",LIGHT_BLUE,my_task_data->subtask[i]->pspid);
			printf("  %s%6i",LIGHT_GREEN,my_task_data->subtask[i]->pstid);
			if(my_task_data->subtask[i]->totaltime==0){
				printf("  %s%6i",RED,my_task_data->subtask[i]->currentcore);
				printf("  %s%5.1f",RED,my_task_data->subtask[i]->pcpu);
				printf("  %s%5.1f%s",LIGHT_CYAN,my_task_data->subtask[i]->pmem,yellow);
				//printf(" %s%9Li us",RED,my_task_data->subtask[i]->starttime);
			}else{
				printf("  %s%9lli ms",NO_COLOUR,my_task_data->subtask[i]->totaltime/1000000);
				printf("  %5.1f",my_task_data->subtask[i]->pmem);
				//printf("  %s%9Li us",NO_COLOUR,my_task_data->subtask[i]->finishtime);
				//printf(" %s%9Li us",NO_COLOUR,my_task_data->subtask[i]->starttime);
			}
			printf("%s ",str_memorysize( my_task_data->subtask[i]->rchar,szBuffer));
			printf("%s ",str_memorysize( my_task_data->subtask[i]->wchar,szBuffer));
			printf("%s ",str_memorysize( my_task_data->subtask[i]->syscr,szBuffer));
			printf("%s ",str_memorysize( my_task_data->subtask[i]->syscw,szBuffer));
			printf("%s ",str_memorysize( my_task_data->subtask[i]->read_bytes,szBuffer));
			printf("%s ",str_memorysize( my_task_data->subtask[i]->write_bytes,szBuffer));
			printf("%s ",str_memorysize( my_task_data->subtask[i]->cancelled_write_bytes,szBuffer));
			printf("\n");
		}else{
			printf(" pid %i tid %i\n",my_task_data->subtask[i]->pspid ,my_task_data->subtask[i]->pstid);
		}
	}
	free(szBuffer);
	return 0;
}


unsigned int save_stats(FILE *fp, int searchprocess, task_data *my_task_data ){
	unsigned int i;
	struct timespec timestamp;
	double timestamp_ms;
	char *szBuffer=(char *)malloc(255*sizeof(char));
	for(i=0;i<my_task_data->totaltid;i++){
		if (searchprocess == my_task_data->subtask[i]->pstid){
			if(my_task_data->subtask[i]->totaltime==0){
				int encontrado=my_task_data->total_user_def;
			// 	encuentra el k para el que los tid coinciden,
				for(int k=0;k<my_task_data->total_user_def;k++){
					if(my_task_data->task_def[k]->pstid==my_task_data->subtask[i]->pstid){
						encontrado=k;
					}
				}
				/*get current timestamp in ms*/
				clock_gettime(CLOCK_REALTIME, &timestamp);
				timestamp_ms = timestamp.tv_sec * 1000.0 + (double)(timestamp.tv_nsec / 1.0e6);
				fprintf(fp, "\"local_timestamp\":\"%.1f\"", timestamp_ms);
				if(encontrado<my_task_data->total_user_def){
					fprintf(fp,", \"component\":\"%s\"", my_task_data->task_def[encontrado]->component_name);
				}
				fprintf(fp,", \"cpu_load\":\"%.2f\"", my_task_data->subtask[i]->pcpu);
				fprintf(fp,", \"mem_load\":\"%.2f\"", my_task_data->subtask[i]->pmem);
				fprintf(fp,", \"rchar\":%s",str_memorysize( my_task_data->subtask[i]->rchar,szBuffer));
				fprintf(fp,", \"wchar\":%s",str_memorysize( my_task_data->subtask[i]->wchar,szBuffer));
				fprintf(fp,", \"syscr\":%s",str_memorysize( my_task_data->subtask[i]->syscr,szBuffer));
				fprintf(fp,", \"syscw\":%s",str_memorysize( my_task_data->subtask[i]->syscw,szBuffer));
				fprintf(fp,", \"read\":%s",str_memorysize( my_task_data->subtask[i]->read_bytes,szBuffer));
				fprintf(fp,", \"write\":%s",str_memorysize( my_task_data->subtask[i]->write_bytes,szBuffer));
				fprintf(fp,", \"cancel_wr\":%s\n",str_memorysize( my_task_data->subtask[i]->cancelled_write_bytes,szBuffer));
// 			}else{ //nothing to save, component finished
// 				printf("  %s%9lli ms",NO_COLOUR,my_task_data->subtask[i]->totaltime/1000000);
				//printf("  %s%9Li us",NO_COLOUR,my_task_data->subtask[i]->finishtime);
				//printf(" %s%9Li us",NO_COLOUR,my_task_data->subtask[i]->starttime);
			}
		}
	// 	if(flag & HAS_IO_STAT) {
			/*calculate the values for disk stats */
	// 		my_task_data->subtask[i]->throughput = (my_task_data->subtask[i]->rchar + my_task_data->subtask[i]->wchar) * 1000.0 / sampling_interval; //in bytes/s
			if(my_task_data->subtask[i]->min_write_bytes > my_task_data->subtask[i]->wchar) my_task_data->subtask[i]->min_write_bytes = my_task_data->subtask[i]->wchar;
			if(my_task_data->subtask[i]->max_write_bytes < my_task_data->subtask[i]->wchar) my_task_data->subtask[i]->max_write_bytes = my_task_data->subtask[i]->wchar;
			if(my_task_data->subtask[i]->min_read_bytes > my_task_data->subtask[i]->rchar) my_task_data->subtask[i]->min_read_bytes = my_task_data->subtask[i]->rchar;
			if(my_task_data->subtask[i]->max_read_bytes < my_task_data->subtask[i]->rchar) my_task_data->subtask[i]->max_read_bytes = my_task_data->subtask[i]->rchar;
	// 	}
	// 	if(flag & HAS_NET_STAT) {
	// 		if(my_task_data->subtask[i]->min_send_bytes > stats->send_bytes) stats->min_send_bytes = stats->send_bytes;
	// 		if(my_task_data->subtask[i]->max_send_bytes < stats->send_bytes) stats->max_send_bytes = stats->send_bytes;
	// 		my_task_data->subtask[i]->accum_send_bytes += stats->send_bytes;
	// 	}
		if(my_task_data->subtask[i]->min_CPU_usage_rate > my_task_data->subtask[i]->pcpu) my_task_data->subtask[i]->min_CPU_usage_rate = my_task_data->subtask[i]->pcpu;
		if(my_task_data->subtask[i]->max_CPU_usage_rate < my_task_data->subtask[i]->pcpu) my_task_data->subtask[i]->max_CPU_usage_rate = my_task_data->subtask[i]->pcpu;
		if(my_task_data->subtask[i]->min_RAM_usage_rate > my_task_data->subtask[i]->pmem) my_task_data->subtask[i]->min_RAM_usage_rate = my_task_data->subtask[i]->pmem;
		if(my_task_data->subtask[i]->max_RAM_usage_rate < my_task_data->subtask[i]->pmem) my_task_data->subtask[i]->max_RAM_usage_rate = my_task_data->subtask[i]->pmem;
	// 	if(my_task_data->subtask[i]->min_swap_usage_rate > my_task_data->subtask[i]->swap_usage_rate) my_task_data->subtask[i]->min_swap_usage_rate = my_task_data->subtask[i]->swap_usage_rate;
	// 	if(my_task_data->subtask[i]->max_swap_usage_rate < my_task_data->subtask[i]->swap_usage_rate) my_task_data->subtask[i]->max_swap_usage_rate = my_task_data->subtask[i]->swap_usage_rate;
		my_task_data->subtask[i]->accum_CPU_usage_rate += my_task_data->subtask[i]->pcpu;
		my_task_data->subtask[i]->accum_RAM_usage_rate += my_task_data->subtask[i]->pmem;
	// 	my_task_data->subtask[i]->accum_swap_usage_rate += my_task_data->subtask[i]->swap_usage_rate;
		my_task_data->subtask[i]->counter=my_task_data->subtask[i]->counter+1;
	}
	free(szBuffer);
	return 0;
}


size_t execute_command(char *command, char *comout, size_t *comalloc){
	// Setup our pipe for reading and execute our command.
	FILE *fd;
	size_t comlen = 0;
	fd = popen(command,"r");//if (!fd) == if(fp==NULL) This if statement implicitly checks "is not 0", so we reverse that to mean "is 0".
	if (fd!=NULL){
		char buffer[256];
		size_t chread;
		/* Use fread so binary data is dealt with correctly */
		while ((chread = fread(buffer, 1, sizeof(buffer), fd)) != 0) {
			if (comlen + chread +1>= *comalloc) {
				*comalloc = *comalloc + *comalloc;
				comout = (char *) realloc(comout, *comalloc * sizeof(char));
			}
			memmove(comout + comlen, buffer, chread);// destination source numbytes
			comlen += chread;
		}
		pclose(fd);
	}
	comout[comlen]='\0';
	return comlen;
}


unsigned int procesa_pid_load(const unsigned int pid, task_data *my_task_data) {
	unsigned int i,contador;
	size_t comalloc = 8256;
	char *comout = (char *) malloc(comalloc * sizeof(char));
	char loadstr[40];
	char command[120];
	for(i=0;i<my_task_data->maxprocesses;i++)
		my_task_data->subtask[i]->updated=false;
	my_task_data->totalpmem=0;
	long long int actual_time=mycurrenttime();
	int j, found;
	int pspid, pstid,currentcore;
	float pcpu, pmem;
// 	if(pid[0]!='\0') {
		// Execute a process listing
		sprintf(command, "ps --no-headers -p %u -L -o pid,tid,psr,pcpu,pmem,size", pid); //stime,time
		size_t comlen = execute_command( command, comout, &comalloc);
		//update all the loads to 0 of all tasks registerd in my_task_data
		//first need to find if the pspid and pstid were already registered in my_task_data, its position will be stored in "index"
		//if not find register, then create a new entry and define the current time as start time
		if(comlen!=0){
			i=0;
			contador=0;
			while((comout[i]!='\0')&&(i<comlen)){
				while (comout[i]==' ')
					i++;
				if(comout[i]!=' '){
					contador++;
					i=process_str(comout, loadstr, i);//loads into the string loadstr the next entry in the input line
					if(contador==1){
						pspid=atoi(loadstr);
					}else if(contador==2) {
						pstid=atoi(loadstr);
					}else if(contador==3) {
						currentcore=atoi(loadstr);
					}else if(contador==4){
						pcpu=atof(loadstr);
						if( pcpu>100.0)
							 pcpu=100.0;//evaluated cases shows few glitches when start running a program. 
					}else if(contador==5){
						pmem=atof(loadstr);
						my_task_data->totalpmem+= pmem;
					} 
				}
				if(comout[i]=='\n'){
					j=0;
					found=false;
					while((j<my_task_data->totaltid)&&(found==false)){
						if((my_task_data->subtask[j]->pspid==pspid)&&(my_task_data->subtask[j]->pstid==pstid)){
							found=true;
						}else{
							j++;
						}
					}
					if((found==false)&&(my_task_data->totaltid<my_task_data->maxprocesses+1)){
						if(pcpu>0.0){//if(pspid==pstid){
							j=my_task_data->totaltid;
							my_task_data->totaltid=my_task_data->totaltid+1;
							my_task_data->subtask[j]->starttime=actual_time;
							my_task_data->subtask[j]->rchar=0;
							my_task_data->subtask[j]->wchar=0;
							my_task_data->subtask[j]->syscr=0;
							my_task_data->subtask[j]->syscw=0;
							my_task_data->subtask[j]->read_bytes=0;
							my_task_data->subtask[j]->write_bytes=0;
							my_task_data->subtask[j]->cancelled_write_bytes=0;
							
							


 	my_task_data->subtask[j]->accum_read_bytes =0;
	my_task_data->subtask[j]->accum_write_bytes =0;
	my_task_data->subtask[j]->before_accum_read_bytes = my_task_data->subtask[j]->accum_read_bytes;
	my_task_data->subtask[j]->before_accum_write_bytes = my_task_data->subtask[j]->accum_write_bytes;
	my_task_data->subtask[j]->counter=1;
 
	my_task_data->subtask[j]->CPU_usage_rate = 0.0;
	my_task_data->subtask[j]->RAM_usage_rate = 0.0;
	my_task_data->subtask[j]->swap_usage_rate = 0.0;
	my_task_data->subtask[j]->send_bytes=0;
	my_task_data->subtask[j]->rcv_bytes=0;
// 	for the case of being uninitialized
	my_task_data->subtask[j]->before_total_cpu_time=my_task_data->subtask[j]->total_cpu_time;
	my_task_data->subtask[j]->before_process_CPU_time=my_task_data->subtask[j]->process_CPU_time;

	my_task_data->subtask[j]->min_CPU_usage_rate = 0;
	my_task_data->subtask[j]->max_CPU_usage_rate = 0;

	my_task_data->subtask[j]->min_RAM_usage_rate = 0;
	my_task_data->subtask[j]->max_RAM_usage_rate = 0;

	my_task_data->subtask[j]->min_swap_usage_rate = 0;
	my_task_data->subtask[j]->max_swap_usage_rate = 0;

	my_task_data->subtask[j]->accum_CPU_usage_rate = 0;
	my_task_data->subtask[j]->accum_RAM_usage_rate = 0;
	my_task_data->subtask[j]->accum_swap_usage_rate = 0;
	
	my_task_data->subtask[j]->min_write_bytes = 0;
	my_task_data->subtask[j]->max_write_bytes = 0;

	my_task_data->subtask[j]->min_read_bytes = 0;
	my_task_data->subtask[j]->max_read_bytes = 0;

	my_task_data->subtask[j]->min_read_bytes = 0;
	my_task_data->subtask[j]->max_read_bytes = 0;
	
	my_task_data->subtask[j]->min_send_bytes = 0;
	my_task_data->subtask[j]->max_send_bytes = 0;
	my_task_data->subtask[j]->accum_send_bytes = 0;
							
							
							if(j==0)
								my_task_data->first_start=actual_time;
							found=true;
						}
					}
					if(found==true){
						my_task_data->subtask[j]->updated=true;
						my_task_data->subtask[j]->pspid=pspid;
						my_task_data->subtask[j]->pstid=pstid;
						my_task_data->subtask[j]->currentcore=currentcore;
						my_task_data->subtask[j]->totaltime=0;
						my_task_data->subtask[j]->pcpu=pcpu;
						my_task_data->subtask[j]->pmem=pmem;
					}
					contador=0;
					i++;
				}
			}
		}
// 	}
	//now we clean the list removing the subtasks not found in the last ps.
	//We update the finishing time
	int count_active_tasks=0;
	int finishing_tasks=0;
	for(i=0;i<my_task_data->totaltid;i++){
		if(my_task_data->subtask[i]->updated==true){ 
			count_active_tasks++;
		}else if(my_task_data->subtask[i]->totaltime==0){
			finishing_tasks++;
			my_task_data->subtask[i]->finishtime=actual_time;
			my_task_data->subtask[i]->totaltime=my_task_data->subtask[i]->finishtime- my_task_data->subtask[i]->starttime;
			my_task_data->subtask[i]->pcpu=0.0;
			my_task_data->subtask[i]->pmem=0.0;
		}
	}	
	if((count_active_tasks==0)&&(finishing_tasks>0))
		my_task_data->last_end=actual_time;
	if(my_task_data->totaltid>my_task_data->maxtotaltid) my_task_data->maxtotaltid=my_task_data->totaltid;
	free(comout);
	return 0;
}

void procesa_task_io(task_data *my_task_data ) {
	char command[120];
	size_t comalloc = 8256;
	char *comout = (char *) malloc(comalloc * sizeof(char));
	int subtask=0;
	char loadstr[250];
	for(subtask=0;subtask< my_task_data->totaltid;subtask++){
		if(my_task_data->subtask[subtask]->updated==true){
			if(my_task_data->subtask[subtask]->pstid==my_task_data->subtask[subtask]->pspid){
				int i=0; 
				sprintf(command, "if [ -e /proc/%i/io ]; then cat /proc/%i/io; fi;",my_task_data->subtask[subtask]->pstid,my_task_data->subtask[subtask]->pstid);
				execute_command(command,comout, &comalloc);
// 				printf("pstid %i: comout is %s\n",my_task_data->subtask[subtask]->pstid,comout);
				if(comout[0]!='\0'){
					while(comout[i]!='\0'){//we need to consume line by line
						i=getline_str(comout, loadstr, i);
// 						printf("loadstr is:%s\n",loadstr);
						find_llint_from_label(loadstr, "rchar: ", &my_task_data->subtask[subtask]->rchar);
						find_llint_from_label(loadstr, "wchar: ", &my_task_data->subtask[subtask]->wchar);
						find_llint_from_label(loadstr, "syscr: ", &my_task_data->subtask[subtask]->syscr);
						find_llint_from_label(loadstr, "syscw: ", &my_task_data->subtask[subtask]->syscw);
						find_llint_from_label(loadstr, "read_bytes: ", &my_task_data->subtask[subtask]->read_bytes);
						find_llint_from_label(loadstr, "write_bytes: ", &my_task_data->subtask[subtask]->write_bytes);
						find_llint_from_label(loadstr, "cancelled_write_bytes: ", &my_task_data->subtask[subtask]->cancelled_write_bytes);
					} //end while
			}
		}}//end if
	}//end for i
	free(comout);
}


void free_mem_report(struct task_data_t *my_task_data_a){
	for(int i=0;i<my_task_data_a->maxprocesses;i++){
		free(my_task_data_a->subtask[i]);
		free(my_task_data_a->task_def[i]);
	}
	free(my_task_data_a->subtask );
	free(my_task_data_a->task_def);
}

void init_stats(task_data *my_task_data_a){
	my_task_data_a->maxprocesses =30;
	my_task_data_a->maxcores=30;
	my_task_data_a->subtask = (struct sub_task_data **) malloc( my_task_data_a->maxprocesses * sizeof(struct sub_task_data *));
	my_task_data_a->task_def = (struct sub_task_user_def **) malloc( my_task_data_a->maxprocesses * sizeof(struct sub_task_user_def *));
	for(int i=0;i<my_task_data_a->maxprocesses;i++){
		my_task_data_a->subtask[i] = (struct sub_task_data *) malloc(sizeof(struct sub_task_data));
		my_task_data_a->task_def[i] = (struct sub_task_user_def *) malloc(sizeof(struct sub_task_user_def));
	}
	my_task_data_a->cores = (struct cores_data *) malloc( my_task_data_a->maxcores * sizeof(struct cores_data));
	my_task_data_a->totaltid=0;
	my_task_data_a->maxtotaltid=1;
	my_task_data_a->first_start=0;
	my_task_data_a->last_end=0;
	for(int i=0;i<my_task_data_a->maxprocesses;i++)
		my_task_data_a->subtask[i]->totaltime=0; // collected metrics
}

 
void stats_sample(const unsigned int pids, task_data *my_task_data) {
	procesa_pid_load(pids, my_task_data);
	procesa_task_io(my_task_data);
}
