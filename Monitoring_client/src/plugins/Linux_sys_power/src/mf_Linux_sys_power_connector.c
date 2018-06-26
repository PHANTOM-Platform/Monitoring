/*
* Copyright (C) 2015-2017 University of Stuttgart
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
#include <unistd.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include "mf_Linux_sys_power_connector.h"
#include <malloc.h>

/***********************************************************************
CPU Specification
- power per core
***********************************************************************/
#define MAX_CPU_POWER 8.23
#define MIN_CPU_POWER 0.75

/***********************************************************************
Memory Specification
***********************************************************************/
#define MEMORY_POWER 3.2 //in Watts, from my memory module specification
#define L2CACHE_MISS_LATENCY 2.09 //ns, get use calibrator
#define L2CACHE_LINE_SIZE 256 //byte get use calibrator

/***********************************************************************
Energy of the disk specs
- Read: 0.02 * 2.78
- Write: 0.02 * 2.19
***********************************************************************/
#define E_DISK_R_PER_KB (0.02 * 2.78)	// milliJoule/KB
#define E_DISK_W_PER_KB  (0.02 * 2.19)	// milliJoule/KB

/***********************************************************************
My Laptop Intel 2200 BG wireless network card:
- Transmit: 1800 mW
- Receive: 1400 mW
- Real upload bandwidth: 12.330M/s
- Real download bandwidth 5.665M/s
***********************************************************************/
#define E_NET_SND_PER_KB (1800 / (1024 * 12.330))	// milliJoule/KB
#define E_NET_RCV_PER_KB (1400 / (1024 * 5.665))	// milliJoule/KB

#define SUCCESS 1
#define FAILURE 0
#define POWER_EVENTS_NUM 5

#define NET_STAT_FILE "/proc/net/dev"
#define IO_STAT_FILE "/proc/%d/io"

#define HAS_CPU_STAT 0x01
#define HAS_NET_STAT 0x02 
#define HAS_RAM_STAT 0x04
#define HAS_IO_STAT 0x08
#define HAS_ALL 0x10

/*******************************************************************************
* Variable Declarations
******************************************************************************/
/* flag indicates which events are given as input */
unsigned int flag = 0;
/* time in seconds */
double before_time, after_time; 

const char Linux_sys_power_metrics[POWER_EVENTS_NUM][32] = {
	"estimated_CPU_power", "estimated_wifi_power",
	"estimated_memory_power", "estimated_disk_power", "estimated_total_power" };

int nr_cpus;
int fd[20];
float CPU_energy_before, CPU_energy_after;
unsigned long long memaccess_before, memaccess_after;

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

/*******************************************************************************
* Forward Declarations
******************************************************************************/
int flag_init(char **events, size_t num_events);
int NET_stat_read(struct net_stats *nets_info);
int sys_IO_stat_read(struct io_stats *total_io_stat);
int process_IO_stat_read(int pid, struct io_stats *io_info);
float sys_net_energy(struct net_stats *stats_before, struct net_stats *stats_after);
float sys_disk_energy(struct io_stats *stats_before, struct io_stats *stats_after);
float CPU_energy_read(void);
void create_perf_stat_counter(void);
unsigned long long read_counter(int fd);
unsigned long long memory_counter_read(void);

/*******************************************************************************
* Functions implementation
******************************************************************************/
/** @brief Initializes the Linux_sys_power plugin
*
*  Check if input events are valid; add valid events to the data->events
*  acquire the previous value and before timestamp
*
*  @return 1 on success; 0 otherwise.
*/
int mf_Linux_sys_power_init(Plugin_metrics *data, char **events, size_t num_events)
{
	/* failed to initialize flag means that all events are invalid */
	if(flag_init(events, num_events) == 0) {
		return FAILURE;
	}
	
	/* initialize Plugin_metrics events' names according to flag */
	int i = 0;

	if(flag & HAS_ALL) {
		data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));	
		strcpy(data->events[i], "estimated_total_power");
		i++;

		/* read the current cpu energy */
		CPU_energy_before = CPU_energy_read();

		/* init perf counter and read the current memory access times */
		create_perf_stat_counter();
		memaccess_before = memory_counter_read();

		/* read the current network rcv/send bytes */
		NET_stat_read(&net_stat_before);

		/* read the current io read/write bytes for all processes */
		sys_IO_stat_read(&io_stat_before);

		/* data->events create if other metrics are required */
		if(flag & HAS_CPU_STAT) {
			data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));	
			strcpy(data->events[i], "estimated_CPU_power");
			i++;
		}
		if(flag & HAS_NET_STAT) {
			data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));	
			strcpy(data->events[i], "estimated_wifi_power");
			i++;
		}
		if((flag & HAS_RAM_STAT) || (flag & HAS_IO_STAT)) {
			data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));	
			strcpy(data->events[i], "estimated_memory_power");
			i++;
			data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));	
			strcpy(data->events[i], "estimated_disk_power");
			i++;
		}
	} else {
		if(flag & HAS_CPU_STAT) {
			data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));	
			strcpy(data->events[i], "estimated_CPU_power");
			i++;
			/* read the current cpu energy */
			CPU_energy_before = CPU_energy_read();
		}
		if(flag & HAS_NET_STAT) {
			data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));	
			strcpy(data->events[i], "estimated_wifi_power");
			i++;
			/* read the current network rcv/send bytes */
			NET_stat_read(&net_stat_before);
		}
		if((flag & HAS_RAM_STAT) || (flag & HAS_IO_STAT)) {
			data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));	
			strcpy(data->events[i], "estimated_memory_power");
			i++;
			/* init perf counter and read the current memory access times */
			create_perf_stat_counter();
			memaccess_before = memory_counter_read();

			data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));	
			strcpy(data->events[i], "estimated_disk_power");
			i++;
			/* read the current io read/write bytes for all processes */
			sys_IO_stat_read(&io_stat_before);
		}
	}

	data->num_events = i;
	
	/* get the before timestamp in second */
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
	before_time = timestamp.tv_sec * 1.0  + (double)(timestamp.tv_nsec / 1.0e9);
	return SUCCESS;
}

/** @brief Samples all possible events and stores data into the Plugin_metrics
*
*  @return 1 on success; 0 otherwise.
*/
int mf_Linux_sys_power_sample(Plugin_metrics *data)
{
	/* get current timestamp in second */
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
	after_time = timestamp.tv_sec * 1.0  + (double)(timestamp.tv_nsec / 1.0e9);

	double time_interval = after_time - before_time; /* get time interval */
	
	float ecpu, emem, enet, edisk;

	int i = 0;
	if(flag & HAS_ALL) {
		/* get current CPU energy (unit in milliJoule) */
		CPU_energy_after = CPU_energy_read();
		
		/* get current memory access counter value */
		memaccess_after = memory_counter_read();

		/* get network statistics */
		NET_stat_read(&net_stat_after);

		/* get disk statistics */
		sys_IO_stat_read(&io_stat_after);

		/* calculate the energy during the interval (unit in milliJoule) */
		ecpu = CPU_energy_after - CPU_energy_before;
		CPU_energy_before = CPU_energy_after;

		emem = ((io_stat_after.read_bytes + io_stat_after.write_bytes - io_stat_before.read_bytes - io_stat_before.write_bytes) 
					/ L2CACHE_LINE_SIZE + (memaccess_after - memaccess_before)) *
					L2CACHE_MISS_LATENCY * MEMORY_POWER * 1.0e-6;
		memaccess_before = memaccess_after;

		enet = sys_net_energy(&net_stat_before, &net_stat_after);

		edisk = sys_disk_energy(&io_stat_before, &io_stat_after);

		/* get total energy via addition; calculate average power during the time interval */
		data->values[i] = (ecpu + emem + enet + edisk) / time_interval;
		i++;
		/* assign values to the data->values according to the flag (unit in mW)*/
		if(flag & HAS_CPU_STAT) {
			data->values[i] = ecpu / time_interval;
			i++;
		}
		if(flag & HAS_NET_STAT) {
			data->values[i] = enet / time_interval;
			i++;
		}
		if((flag & HAS_RAM_STAT) || (flag & HAS_IO_STAT)) {
			data->values[i] = emem / time_interval;
			i++;
			data->values[i] = edisk / time_interval;
			i++;
		}
	} else {
		if(flag & HAS_CPU_STAT) {
			CPU_energy_after = CPU_energy_read();
			ecpu = CPU_energy_after - CPU_energy_before;
			CPU_energy_before = CPU_energy_after;
			data->values[i] = ecpu / time_interval;
			i++;
		}
		if(flag & HAS_NET_STAT) {
			/* get net energy (unit in milliJoule) */
			NET_stat_read(&net_stat_after);
			enet = sys_net_energy(&net_stat_before, &net_stat_after);			
			data->values[i] = enet / time_interval;
			i++;
		}
		if((flag & HAS_RAM_STAT) || (flag & HAS_IO_STAT)) {
			memaccess_after = memory_counter_read();
			sys_IO_stat_read(&io_stat_after);

			emem = ((io_stat_after.read_bytes + io_stat_after.write_bytes - io_stat_before.read_bytes - io_stat_before.write_bytes) 
						/ L2CACHE_LINE_SIZE + (memaccess_after - memaccess_before)) *
						L2CACHE_MISS_LATENCY * MEMORY_POWER * 1.0e-6;
			memaccess_before = memaccess_after;
			data->values[i] = emem / time_interval;
			i++;

			edisk = sys_disk_energy(&io_stat_before, &io_stat_after);
			data->values[i] = edisk / time_interval;
			i++;
		}
	}

	/* update timestamp */
	before_time = after_time;
	return SUCCESS;
}

/** @brief Formats the sampling data into a json string
*
*  json string contains: plugin name, timestamps, metrics_name and metrics_value
*
*/
void mf_Linux_sys_power_to_json(Plugin_metrics *data, char *json)
{
	char tmp[128] = {'\0'};
	int i;
	/*
	* prepares the json string, including current timestamp, and name of the plugin
	*/
	sprintf(json, "\"type\":\"Linux_sys_power\"");
	sprintf(tmp, ",\"local_timestamp\":\"%.1f\"", after_time * 1.0e3);
	strcat(json, tmp);

	/*
	* filters the sampled data with respect to metrics values
	*/
	for (i = 0; i < data->num_events; i++) {
		/* if metrics' name is power_CPU, but flag do not HAS_CPU_STAT, ignore the value*/
		if(strcmp(data->events[i], "estimated_memory_power") == 0 && !(flag & HAS_RAM_STAT))
			continue;

		/* if metrics' name is power_mem, but flag do not HAS_RAM_STAT, ignore the value*/
		if(strcmp(data->events[i], "estimated_disk_power") == 0 && !(flag & HAS_IO_STAT))
			continue;

		/* if metrics' value >= 0.0, append the metrics to the json string */
		if(data->values[i] >= 0.0) {
			sprintf(tmp, ",\"%s\":%.3f", data->events[i], data->values[i]);
			strcat(json, tmp);
		}
	}
}


/* Adds events to the data->events, if the events are valid */
int flag_init(char **events, size_t num_events) 
{
	int i, ii;
	for (i=0; i < num_events; i++) {
		for (ii = 0; ii < POWER_EVENTS_NUM; ii++) {
			/* if events name matches */
			if(strcmp(events[i], Linux_sys_power_metrics[ii]) == 0) {
				/* get the flag updated */
				unsigned int current_event_flag = 1 << ii;
				flag = flag | current_event_flag;
			}
		}
	}
	if (flag == 0) {
		fprintf(stderr, "Wrong given metrics.\nPlease given metrics ");
		for (ii = 0; ii < POWER_EVENTS_NUM; ii++) {
			fprintf(stderr, "%s ", Linux_sys_power_metrics[ii]);
		}
		fprintf(stderr, "\n");
		return FAILURE;
	} else {
		return SUCCESS;
	}
}

/* Gets current network stats (send and receive bytes via wireless card). */
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

/* Gets the IO stats of the whole system (read IO stats for all processes and make an addition) */
int sys_IO_stat_read(struct io_stats *total_io_stat) {
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

	/* reset total_io_stat into zeros */
	total_io_stat->read_bytes = 0;
	total_io_stat->write_bytes = 0;

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

/* Gets the IO stats of a specified process (parameters are pid and io_info) */
int process_IO_stat_read(int pid, struct io_stats *io_info) {
	FILE *fp;
	char filename[128], line[256];

	sprintf(filename, IO_STAT_FILE, pid);
	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", filename);
		return FAILURE;
	}
	io_info->read_bytes = 0;
	io_info->write_bytes = 0;

	while (fgets(line, 256, fp) != NULL) {
		if (!strncmp(line, "read_bytes:", 11)) {
			sscanf(line + 12, "%llu", &io_info->read_bytes);
		}
		if (!strncmp(line, "write_bytes:", 12)) {
			sscanf(line + 13, "%llu", &io_info->write_bytes);
		}
		if ((io_info->read_bytes * io_info->write_bytes) != 0) {
			break;
		}
	}
	fclose(fp);
	return SUCCESS;
}

/* Calculates the system network energy consumption; updates net statistics values; return the network energy consumption (in milliJoule) */
float sys_net_energy(struct net_stats *stats_before, struct net_stats *stats_after) 
{
	unsigned long long rcv_bytes, send_bytes;

	rcv_bytes = stats_after->rcv_bytes - stats_before->rcv_bytes;
	send_bytes = stats_after->send_bytes - stats_before->send_bytes;

	float enet = ((rcv_bytes * E_NET_RCV_PER_KB) + (send_bytes * E_NET_SND_PER_KB)) / 1024;
	
	/* update the net_stat_before values by the current values */
	stats_before->rcv_bytes = stats_after->rcv_bytes;
	stats_before->send_bytes = stats_after->send_bytes;
	return enet;
}

/* Calculates the system disk energy consumption; updates io statistics values; return the disk energy consumption (in milliJoule) */
float sys_disk_energy(struct io_stats *stats_before, struct io_stats *stats_after) 
{
	unsigned long long read_bytes, write_bytes;

	read_bytes = stats_after->read_bytes - stats_before->read_bytes;
	write_bytes = stats_after->write_bytes - stats_before->write_bytes;

	float edisk = ((read_bytes * E_DISK_R_PER_KB) + (write_bytes * E_DISK_W_PER_KB)) / 1024;

	/* update the io_stat_before values by the current values */
	stats_before->read_bytes = stats_after->read_bytes;
	stats_before->write_bytes = stats_after->write_bytes;
	return edisk;
}

//Function for increase dynamically a string concatenating strings at the end
//It free the memory of the first pointer if not null
char* concat_and_free(char **s1, const char *s2)
{
	char *result = NULL;
	unsigned int new_lenght= strlen(s2)+1; //+1 for the null-terminator;
	if(*s1 != NULL){
		new_lenght+= strlen(*s1);//current lenght
		if(new_lenght> malloc_usable_size(*s1)){
			result = (char *) malloc(new_lenght);
			strcpy(result, *s1);
			free(*s1);  
		}else{
			result = *s1; 
		}
	}else{
		result = malloc(new_lenght);
		result[0]='\0';
	}
	//in real code you would check for errors in malloc here 
	strcat(result, s2);
	return result;
}

/* get the cpu freq counting; return the cpu energy since the system's last booting */
float CPU_energy_read(void) 
{
	/* read the system cpu energy based on given max- and min- cpu energy, and frequencies statistics */
	FILE *fp;
	char line[32] = {'\0'};
	DIR *dir;
	int i, max_i;
	struct dirent *dirent;
	char *cpu_freq_file = NULL; 
	
	float energy_each, energy_total;
	unsigned long long tmp;
	unsigned long long freqs[16];

	/* check if system support cpu freq statistics */
	fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/stats/time_in_state", "r");
	if(fp == NULL) {
		printf("ERROR: CPU frequency statistics are not supported.\n");
		return 0.0;
	}

	energy_total = 0.0;
	float power_range = MAX_CPU_POWER - MIN_CPU_POWER;

	dir = opendir("/sys/devices/system/cpu");
	if(!dir) {
		printf("ERROR: Could not open directory /sys/devices/system/cpu\n");
		return 0.0;
	}

	while ((dirent = readdir(dir))) {
		/* for each entry name starting by cpuxx */
		if (strncmp(dirent->d_name,"cpu", 3) != 0)
			continue;
		cpu_freq_file=concat_and_free(&cpu_freq_file, "/sys/devices/system/cpu/");
		cpu_freq_file=concat_and_free(&cpu_freq_file, dirent->d_name);
		cpu_freq_file=concat_and_free(&cpu_freq_file, "/cpufreq/stats/time_in_state");
		fp = fopen(cpu_freq_file, "r");

		if(!fp)
			continue;

		for (i = 0; !feof(fp) && (i <= 15); i++) {
			if(fgets(line, 32, fp) == NULL)
				break;
			sscanf(line, "%llu %llu", &tmp, &freqs[i]);
			/* each line has a pair like "<frequency> <time>", which means this CPU spent <time> usertime at <frequency>.
			unit of <time> is 10ms
			*/
		}
		max_i = i - 1;
		fclose(fp);

		for (i = 0; i <= max_i; i++) {
			energy_each = (MAX_CPU_POWER - power_range * i / max_i) * freqs[i] * 10.0; // in milliJoule
			energy_total += energy_each; 
		}
	}

	if(cpu_freq_file!= NULL ) free(cpu_freq_file);
	closedir(dir);
	return energy_total;
}

/* init perf counter for hardware cache misses; get the file descriptors for all CPUs */
void create_perf_stat_counter(void)
{
	/* get the nubmer of CPUs */
	nr_cpus = sysconf(_SC_NPROCESSORS_ONLN);

	struct perf_event_attr attr; //hardware L2 cache miss
	memset(&attr, 0, sizeof(struct perf_event_attr));
	attr.type =	PERF_TYPE_HARDWARE; 
	attr.config = PERF_COUNT_HW_CACHE_MISSES;
	attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
	attr.inherit = 1;
	attr.disabled = 0;
	attr.enable_on_exec = 1;
	attr.size = sizeof(attr);

	/* The counter measures the CPU L2 cache misses; return the file descriptor for the counter */
	int i;
	for (i = 0; i < nr_cpus; i++) {
		fd[i] = syscall(__NR_perf_event_open, &attr, -1, i, -1, 0);
	}
}

/* read the perf counter with given file descriptor */
unsigned long long read_counter(int fd)
{
	unsigned long long single_count[3];
	size_t res;

	if (fd <= 0)
		return 0;

	res = read(fd, single_count, 3 * sizeof(unsigned long long));

	if(res == 3 * sizeof(unsigned long long)) {
		return single_count[0];
	}else {
		return 0;
	}
}

/* read and add all memory counters for each CPU */
unsigned long long memory_counter_read(void) 
{
	int i;
	unsigned long long result;
	for (i = 0; i < nr_cpus; i++) {
		result += read_counter(fd[i]);
	}
	return result;
}
