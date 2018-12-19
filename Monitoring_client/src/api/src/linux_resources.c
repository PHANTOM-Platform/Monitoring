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
#include "util.h"

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
	stats_now->before_process_CPU_time = stats_now->process_CPU_time ;
	stats_now->process_CPU_time = pid_utime + pid_stime;
	fclose(fp);
	return SUCCESS;
}


/** @brief Gets ram usage rate (unit is %) from RAM_STAT_FILE=/proc/meminfo  
 * collected metrics on MemTotal and MemFree
 @return  e */
int RAM_usage_rate_read(struct resources_stats_t *stats_now) {
	FILE *fp; 
	char line[1024]; 
// 	unsigned long  MemFree = 0;
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
 @return  e */
int RAM_process_usage_rate_read(const int pid, struct resources_stats_t *stats_now) {
	FILE *fp;
	char line[1024] ;
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
*  acquire the previous value and before timestamp
*  @return SUCCESS on success; FAILURE otherwise.
*/
int linux_resources_init(int pid, char **events, size_t num_events, struct resources_stats_t stats_now) {
	/* failed to initialize flag means that all events are invalid */
	if(flag_init(events, num_events) == 0)
		return FAILURE;
	/* get the before timestamp in second */
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
// 	before_time = timestamp.tv_sec * 1.0  + (double)(timestamp.tv_nsec / 1.0e9);
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
*  @return SUCCESS on success; FAILURE otherwise.
*/
int linux_resources_sample(const int pid, struct resources_stats_t stat_after ) {
	/* get current timestamp in second */
	struct timespec timestamp;
	int i;
	clock_gettime(CLOCK_REALTIME, &timestamp);
// 	after_time = timestamp.tv_sec * 1.0  + (double)(timestamp.tv_nsec / 1.0e9);
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

int linux_resources(const int pid, char *DataPath, long sampling_interval) {
	char FileName[256];
	int i;
	struct timespec timestamp;
	double timestamp_ms;
	int num_events = RESOURCES_EVENTS_NUM;
	struct resources_stats_t *stats =(struct resources_stats_t *) malloc((num_events)* sizeof(struct resources_stats_t ));

	stats->accum_read_bytes =0;
	stats->accum_write_bytes =0;
	stats->before_accum_read_bytes = stats->accum_read_bytes;
	stats->before_accum_write_bytes = stats->accum_write_bytes;
		
	long int counter=1;
	
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
	stats->before_total_cpu_time=stats->total_cpu_time ;
	stats->before_process_CPU_time=stats->process_CPU_time ;

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
    	timestamp_ms = timestamp.tv_sec * 1000.0  + (double)(timestamp.tv_nsec / 1.0e6);

    	fprintf(fp, "\"local_timestamp\":\"%.1f\"", timestamp_ms );
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
		counter++;
		fprintf(fp, "\n");
	}
	fclose(fp);




	sprintf(FileName, "%s/stats_%s", DataPath, METRIC_NAME_1);
	fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		return 0;
	}
	fprintf(fp, "\"local_timestamp\":\"%.1f\"", timestamp_ms);
	fprintf(fp, ",[{" );
// 	----------------------CPU
		fprintf(fp, ",\"%s\":{", "cpu_usage_rate" );
		fprintf(fp, ",\"count\":\"%li\"", counter);
		fprintf(fp, ",\"min\":\"%.2f\"", stats->min_CPU_usage_rate);
		fprintf(fp, ",\"max\":\"%.2f\"", stats->max_CPU_usage_rate);
		fprintf(fp, ",\"avg\":\"%.2f\"", (float) stats->accum_CPU_usage_rate/ (float)counter);
		fprintf(fp, ",\"sum\":\"%.lli\"},", stats->accum_CPU_usage_rate);
// 	----------------------RAM
		fprintf(fp, ",\"%s\":{", "ram_usage_rate");
		fprintf(fp, ",\"count\":\"%li\"", counter);
		fprintf(fp, ",\"min\":\"%.2f\"", stats->min_RAM_usage_rate);
		fprintf(fp, ",\"max\":\"%.2f\"", stats->max_RAM_usage_rate);
		fprintf(fp, ",\"avg\":\"%.2f\"", (float)stats->accum_RAM_usage_rate/(float)counter);
		fprintf(fp, ",\"sum\":\"%.lli\"},", stats->accum_RAM_usage_rate);
// 	----------------------SWAP
		fprintf(fp, ",\"%s\":{", "swap_usage_rate");
		fprintf(fp, ",\"count\":\"%li\"", counter);
		fprintf(fp, ",\"min\":\"%.2f\"", stats->min_swap_usage_rate);
		fprintf(fp, ",\"max\":\"%.2f\"", stats->max_swap_usage_rate);
		fprintf(fp, ",\"avg\":\"%.2f\"", (float)stats->accum_swap_usage_rate/(float)counter);
		fprintf(fp, ",\"sum\":\"%.lli\"},", stats->accum_swap_usage_rate);
// 	---------------------- IO
		fprintf(fp, ",\"%s\":{", "write_bytes");
		fprintf(fp, ",\"count\":\"%li\"", counter);
		fprintf(fp, ",\"min\":\"%lli\"", stats->min_write_bytes);
		fprintf(fp, ",\"max\":\"%lli\"", stats->max_write_bytes);
			fprintf(fp, ",\"avg\":\"%lli\"", stats->accum_write_bytes/counter);
		fprintf(fp, ",\"sum\":\"%lli\"},", stats->accum_write_bytes);
// ------------------------ IO
		fprintf(fp, ",\"%s\":{", "read_bytes");
		fprintf(fp, ",\"count\":\"%li\"", counter);
		fprintf(fp, ",\"min\":\"%lli\"", stats->min_read_bytes);
		fprintf(fp, ",\"max\":\"%lli\"", stats->max_read_bytes);
			fprintf(fp, ",\"avg\":\"%lli\"", stats->accum_read_bytes/counter);
		fprintf(fp, ",\"sum\":\"%lli\"},", stats->accum_read_bytes);
// -------------------------NET
		fprintf(fp, ",\"%s\":{", "send_bytes");
		fprintf(fp, ",\"count\":\"%li\"", counter);
		fprintf(fp, ",\"min\":\"%lli\"", stats->min_send_bytes);
		fprintf(fp, ",\"max\":\"%lli\"", stats->max_send_bytes);
			fprintf(fp, ",\"avg\":\"%lli\"", stats->accum_send_bytes/counter);
		fprintf(fp, ",\"sum\":\"%lli\"}", stats->accum_send_bytes);
 
// 			nets_info->rcv_bytes += temp_rcv_bytes;
// 			nets_info->send_bytes += temp_send_bytes;		
		
		
	fprintf(fp, "}]\n" );
	fclose(fp);


	free(stats);

	for(i=0;i<num_events;i++)
		free(events[i]);
	free(events);
	return SUCCESS;
}
