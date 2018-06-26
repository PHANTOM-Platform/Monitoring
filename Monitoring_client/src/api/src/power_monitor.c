#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
//#include <linux/hw_breakpoint.h>
#include "power_monitor.h"
#include "mf_api.h"

/**********************************************************
* FUNCTION DECLARATIONS
*****************************************************/
#include <malloc.h>
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
		result = (char *) malloc(new_lenght);
		result[0]='\0';
	}
	//in real code you would check for errors in malloc here 
	strcat(result, s2);
	return result;
}


int power_monitor(int pid, char *DataPath, long sampling_interval)
{
	/*create and open the file*/
	char *FileName=NULL;
	FileName=concat_and_free(&FileName, DataPath);
	FileName=concat_and_free(&FileName, "/");
	FileName=concat_and_free(&FileName, METRIC_NAME_3);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		free(FileName);
		return(0);
	}
	struct timespec timestamp_before, timestamp_after;
	double timestamp_ms;
	float duration, sys_cpu_power, pid_cpu_power, pid_mem_power, pid_disk_power;
	pid_stats_info before, after, delta;
	int fd = create_perf_stat_counter(pid);
	if(fd <= 0){
		free(FileName);
		return 0;
	}

	if(read_and_check(fd, pid, &before) <= 0){
		free(FileName);
		return 0;
	}

	/*in a loop do data sampling and write into the file*/
	while(running) {
		/*get before timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp_before);

		usleep(sampling_interval * 1000);
		
		if(read_and_check(fd, pid, &after) <= 0){
			free(FileName);
			return 0;
		}
	
		/*get after timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp_after);

		/*calculate the increments of counters; update the values before and after */
		if(calculate_and_update(&before, &after, &delta) <= 0)
			continue;
		/* calculate the time interval in seconds */
		duration = timestamp_after.tv_sec - timestamp_before.tv_sec + ((timestamp_after.tv_nsec - timestamp_before.tv_nsec) / 1.0e9);
		/* system-wide cpu power in milliwatt */
		sys_cpu_power = delta.sys_cpu_energy * delta.sys_runtime / (delta.sys_itv * duration);
		/* pid-based cpu power in milliwatt */
		pid_cpu_power = sys_cpu_power * delta.pid_runtime / delta.sys_runtime;
		/* pid-based memory access power in milliwatt */
		pid_mem_power = ((delta.pid_read_bytes + delta.pid_write_bytes - delta.pid_cancelled_writes) / parameters_value[4] + delta.pid_l2_cache_misses) *
						parameters_value[3] * parameters_value[2] * 1.0e-6 / duration;
		/* pid-based disk access power in milliwatt */
		pid_disk_power = (delta.pid_read_bytes * parameters_value[5] + (delta.pid_write_bytes - delta.pid_cancelled_writes) * parameters_value[6]) / 
						(1024 * duration);

		timestamp_ms = timestamp_after.tv_sec * 1000.0  + (double)(timestamp_after.tv_nsec / 1.0e6);
			
		fprintf(fp, "\"local_timestamp\":\"%.1f\", \"%s\":%.3f, \"%s\":%.3f, \"%s\":%.3f, \"%s\":%.3f\n", timestamp_ms, 
				"total_CPU_power", sys_cpu_power,
				"process_CPU_power", pid_cpu_power,
				"process_mem_power", pid_mem_power,
				"process_disk_power", pid_disk_power);
	}
	/*close the file*/
	fclose(fp);
	free(FileName);
	return 1;
}

/* init perf counter for hardware cache misses 
return the file descriptor for further read operations */
int create_perf_stat_counter(int pid)
{
	struct perf_event_attr attr; //cache miss
	memset(&attr, 0, sizeof(struct perf_event_attr));
	attr.type =	PERF_TYPE_HARDWARE;
	attr.config = PERF_COUNT_HW_CACHE_MISSES;
	attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
	attr.inherit = 1;
	attr.disabled = 0;
	attr.enable_on_exec = 1;
	attr.size = sizeof(attr);
	
	/* This measures the specified process/thread on any CPU;
	return the file descriptor for the counter */	
	return syscall(__NR_perf_event_open, &attr, pid, -1, -1, 0);
}

/* read from /proc filesystem all statistics;
get the cpu energy consumption based on cpu freq statistics;
read from perf counter the hardware cache misses */
int read_and_check(int fd, int pid, pid_stats_info *info) 
{
	if(read_pid_time(pid, info) <= 0)
		return 0;

	if(read_pid_io(pid, info) <=0)
		return 0;
	
	if(read_sys_time(info) <= 0)
		return 0;
	
	if(cpu_freq_stat(info) <= 0)
		return 0;
	
	info->pid_l2_cache_misses = read_perf_counter(fd);
	if(info->pid_l2_cache_misses <= 0)
		return 0;

	return 1;
}

/* check if values are increasing; calculate the differences in the time interval; update the before values with after values */
int calculate_and_update(pid_stats_info *before, pid_stats_info *after, pid_stats_info *delta)
{
	if (after->sys_itv <= before->sys_itv)
		return 0;
	if (after->sys_runtime <= before->sys_runtime)
		return 0;

	delta->sys_itv = after->sys_itv - before->sys_itv;
	delta->sys_runtime = after->sys_runtime - before->sys_runtime;
	delta->pid_runtime = after->pid_runtime - before->pid_runtime;
	delta->pid_read_bytes = after->pid_read_bytes - before->pid_read_bytes;
	delta->pid_write_bytes = after->pid_write_bytes - before->pid_write_bytes;
	delta->pid_cancelled_writes = after->pid_cancelled_writes - before->pid_cancelled_writes;
	delta->pid_l2_cache_misses = after->pid_l2_cache_misses - before->pid_l2_cache_misses;
	delta->sys_cpu_energy = after->sys_cpu_energy - before->sys_cpu_energy;

	memcpy(before, after, sizeof(pid_stats_info));
	return 1;
}

/* read the process runtime from /proc/[pid]/stat */
int read_pid_time(int pid, pid_stats_info *info)
{
	FILE *fp;
	char *line= (char *) malloc(1024);
	char pid_cpu_file[128] = {'\0'};
	char tmp_str[32];
	char tmp_char;
	unsigned long long tmp, pid_utime, pid_stime;	

	sprintf(pid_cpu_file, "/proc/%d/stat", pid);
	fp = fopen(pid_cpu_file, "r");

	if(fp == NULL) {
		printf("ERROR: Could not open file %s\n", pid_cpu_file);
		return 0;
	}
	if(fgets(line, 1024, fp) != NULL) {
		sscanf(line, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu",
			(int *)&tmp, tmp_str, &tmp_char, (int *)&tmp, (int *)&tmp, (int *)&tmp, (int *)&tmp, (int *)&tmp, 
			(unsigned int *)&tmp, (unsigned long *)&tmp, (unsigned long *)&tmp, (unsigned long *)&tmp, 
			(unsigned long *)&tmp, (unsigned long *)&pid_utime, (unsigned long *)&pid_stime);
	}

	info->pid_runtime = pid_utime + pid_stime;
	fclose(fp);
	if(line!=NULL) free(line);
	return 1;
}

/* read the process read_bytes, write_bytes, and cancelled_writes from /proc/[pid]/io */
int read_pid_io(int pid, pid_stats_info *info)
{
	FILE *fp;
	char line[128];
	char disk_file[128] = {'\0'};

	sprintf(disk_file, "/proc/%d/io", pid);
	fp = fopen(disk_file, "r");
	
	if(fp == NULL) {
		printf("ERROR: Could not open file %s\n", disk_file);
		return 0;
	}
	while (fgets(line, 128, fp) != NULL) {
		if (!strncmp(line, "read_bytes:", 11)) {
			sscanf(line + 12, "%llu", &info->pid_read_bytes);
		} else if (!strncmp(line, "write_bytes:", 12)) {
			sscanf(line + 13, "%llu", &info->pid_write_bytes);
		} else if (!strncmp(line, "cancelled_write_bytes:", 22)) {
			sscanf(line + 23, "%llu", &info->pid_cancelled_writes);
		}
	}
	fclose(fp);
	return 1;
}

/* read the system itv and runtime from /proc/stat*/
int read_sys_time(pid_stats_info *info)
{
	FILE *fp;
	char *line= (char *) malloc(1024);
	const char cpu_file[] = "/proc/stat";
	unsigned long long cpu_user, cpu_nice, cpu_sys, cpu_idle, cpu_iowait, cpu_hardirq, cpu_softirq, cpu_steal;

	fp = fopen(cpu_file, "r");

	if(fp == NULL) {
		printf("ERROR: Could not open file %s\n", cpu_file);
		return 0;
	}
	if(fgets(line, 1024, fp) != NULL) {
		sscanf(line+5, "%llu %llu %llu %llu %llu %llu %llu %llu",
			&cpu_user,
			&cpu_nice,
			&cpu_sys,
			&cpu_idle,
			&cpu_iowait,
			&cpu_hardirq,
			&cpu_softirq,
			&cpu_steal);
	}
	info->sys_itv = cpu_user + cpu_nice + cpu_sys + cpu_idle + cpu_iowait + cpu_hardirq + cpu_softirq + cpu_steal;
	info->sys_runtime = cpu_user + cpu_sys;
	fclose(fp);
	if(line!=NULL) free(line);
	return 1;
}

/* get the cpu freq counting and return the cpu energy since the last call of the function */
int cpu_freq_stat(pid_stats_info *info) 
{
	/*
	read the system cpu energy based on given max- and min- cpu energy, and frequencies statistics
	*/
	FILE *fp;
	char line[32] = {'\0'};
	DIR *dir;
	int i, max_i;
	struct dirent *dirent;
	char *cpu_freq_file = NULL;
	
	float energy_each, energy_total;
	unsigned long long tmp;
	unsigned long long freqs[16];

	/*
	check if system support cpu freq counting 
	*/
	fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/stats/time_in_state", "r");
	if(fp == NULL) {
		printf("ERROR: CPU frequency statistics are not supported.\n");
		return 0;
	}

	energy_total = 0.0;
	float power_range = parameters_value[0] - parameters_value[1];

	dir = opendir("/sys/devices/system/cpu");
	if(!dir) {
		printf("ERROR: Could not open directory /sys/devices/system/cpu\n");
		return 0;
	}

	while ((dirent = readdir(dir))) {
		/* for each entry name starting by cpuxx */
		if (strncmp(dirent->d_name,"cpu", 3) != 0)
			continue;
// 		sprintf(cpu_freq_file, "/sys/devices/system/cpu/%s/cpufreq/stats/time_in_state", dirent->d_name);
		cpu_freq_file=concat_and_free(&cpu_freq_file, "/sys/devices/system/cpu/");
		cpu_freq_file=concat_and_free(&cpu_freq_file, dirent->d_name);
		cpu_freq_file=concat_and_free(&cpu_freq_file, "/cpufreq/stats/time_in_state");
		fp = fopen(cpu_freq_file, "r");
		cpu_freq_file[0]='\0';//in the next loop the string will be empty
		//memset(cpu_freq_file, '\0', malloc_usable_size(cpu_freq_file));
		
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
			energy_each = (parameters_value[0] - power_range * i / max_i) * freqs[i] * 10.0; // in milliJoule
			energy_total += energy_each; 
		}
	}

	if(cpu_freq_file!= NULL ) free(cpu_freq_file);
	closedir(dir);

	info->sys_cpu_energy = energy_total;
	return 1;
}

/* read the perf counter from the file descriptor */
unsigned long long read_perf_counter(int fd)
{
	unsigned long long single_count[3];
	size_t res;

	if (fd <= 0)
		return 0;

	res = read(fd, single_count, 3 * sizeof(unsigned long long));

	if(res == 3 * sizeof(unsigned long long)) {
		return single_count[0];
	} else {
		return 0;
	}
}
