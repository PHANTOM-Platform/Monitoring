#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "resources_monitor.h"
#include "mf_api.h"

/*******************************************************************************
 * Implementaion
 ******************************************************************************/
int resources_monitor(int pid, char *DataPath, long sampling_interval)
{
	/*create and open the file*/
	char FileName[256] = {'\0'};
	sprintf(FileName, "%s/%s", DataPath, METRIC_NAME_1);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		exit(0);
	}
	struct timespec timestamp;
	double timestamp_ms;
	resources_cpu before, after;
	resources_stats stats;
	resources_stat_cpu(pid, &before);

	/*in a loop do data sampling and write into the file*/
	while(running) {
		usleep(sampling_interval * 1000);
		resources_stat_cpu(pid, &after);
		resources_stat_all_and_calculate(pid, &before, &after, &stats);
		/*get current timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp);
    	timestamp_ms = timestamp.tv_sec * 1000.0  + (double)(timestamp.tv_nsec / 1.0e6);
    	fprintf(fp, "\"local_timestamp\":\"%.1f\", \"%s\":%.3f, \"%s\":%.3f, \"%s\":%.3f\n", timestamp_ms, 
    		"CPU_usage_rate", stats.CPU_usage_rate,
    		"RAM_usage_rate", stats.RAM_usage_rate,
    		"swap_usage_rate", stats.swap_usage_rate);
	}
	/*close the file*/
	fclose(fp);
	return 1;
}

int resources_stat_cpu(int pid, resources_cpu *stats_now)
{
	FILE *fp;
	char line[1024];
	unsigned long long tmp;

	/*read cpu user time and system time from /proc/[pid]/stat */
	char pid_cpu_file[128] = {'\0'};
	unsigned long long pid_utime, pid_stime;

	sprintf(pid_cpu_file, "/proc/%d/stat", pid);
	fp = fopen(pid_cpu_file, "r");
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
	stats_now->process_CPU_time = pid_utime + pid_stime;
	fclose(fp);

	/*read cpu user time and system time from /proc/stat */
	char cpu_file[128] = "/proc/stat";
	unsigned long long cpu_user, cpu_sys;

	fp = fopen(cpu_file, "r");
	if(fp == NULL) {
		printf("ERROR: Could not open file %s\n", cpu_file);
		exit(0);
	}
	if(fgets(line, 1024, fp) != NULL) {
		sscanf(line+5, "%llu %llu %llu %llu %llu %llu %llu %llu",
			&cpu_user, &tmp, &cpu_sys, &tmp, &tmp, &tmp, &tmp, &tmp);
	}
	stats_now->global_CPU_time = cpu_user + cpu_sys;
	fclose(fp);
	return 1;
}

int resources_stat_all_and_calculate(int pid, resources_cpu *before, resources_cpu *after, resources_stats *result)
{
	FILE *fp;
	char line[1024] = {'\0'};
	
	/*read VmRSS and VmSwap from /proc/[pid]/status */
	char pid_status_file[128] = {'\0'};
	unsigned long pid_VmRSS, pid_VmSwap;
	sprintf(pid_status_file, "/proc/%d/status", pid);
	fp = fopen(pid_status_file, "r");
	if(fp == NULL) {
		printf("ERROR: Could not open file %s\n", pid_status_file);
		exit(0);
	}
	while(fgets(line, 1024, fp) != NULL) {
		if (!strncmp(line, "VmRSS:", 6)) {
			sscanf(line + 6, "%lu", &pid_VmRSS);
		}
		if (!strncmp(line, "VmSwap:", 7)) {
			sscanf(line + 7, "%lu", &pid_VmSwap);
		}
	}
	fclose(fp);

	/*read MemTotal and SwapTotal from /proc/meminfo */
	char meminfo_file[128] = "/proc/meminfo";
	unsigned long MemTotal, SwapTotal;
	fp = fopen(meminfo_file, "r");
	if(fp == NULL) {
		printf("ERROR: Could not open file %s\n", meminfo_file);
		exit(0);
	}
	while(fgets(line, 1024, fp) != NULL) {
		if (!strncmp(line, "MemTotal:", 9)) {
			sscanf(line + 9, "%lu", &MemTotal);
		}
		if (!strncmp(line, "SwapTotal:", 10)) {
			sscanf(line + 10, "%lu", &SwapTotal);
		}
	}
	fclose(fp);

	/*calculate for the resources_stats */
	if((after->process_CPU_time <= before->process_CPU_time) || (after->global_CPU_time <= before->global_CPU_time)) {
		result->CPU_usage_rate = 0.0;
	}
	else {
		result->CPU_usage_rate = (after->process_CPU_time - before->process_CPU_time) * 100.0 / 
						(after->global_CPU_time - before->global_CPU_time);	
	}
	
	result->RAM_usage_rate = pid_VmRSS * 100.0 /MemTotal;
	result->swap_usage_rate = pid_VmSwap * 100.0 /SwapTotal;

	/*replace the cpu time*/
	before->process_CPU_time = after->process_CPU_time;
	before->global_CPU_time = after->global_CPU_time;
	return 1;
}
