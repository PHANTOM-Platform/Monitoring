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
#include <unistd.h> //file access
#include <dirent.h>
#include <time.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
//#include <linux/hw_breakpoint.h>
#include "linux_sys_power.h"
#include "linux_resources.h"
#include "mf_util.h"
#include "mf_api.h" //shared variable running
/**********************************************************
* FUNCTION DECLARATIONS
*****************************************************/
#include <malloc.h>

#define SUCCESS 0
#define FAILURE 1

float parameters_value[9];

/* init perf counter for hardware cache misses
return the file descriptor for further read operations */
int create_perf_stat_counter(int pid) {
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
	// pid == -1 and cpu == -1 is an invalid option
	// pid > 0 and cpu == -1  measures the specified process/thread on any CPU.
	//pid == -1 and cpu >= 0 measures all processes/threads on the specified CPU.
	//               last one requires CAP_SYS_ADMIN capability or a
	//               last one: /proc/sys/kernel/perf_event_paranoid value of less than 1.
	const int mcpu = -1;
	const int mgroup_fd = -1;
	const unsigned int mflags=0;
	return syscall(__NR_perf_event_open, &attr, pid, mcpu, mgroup_fd, mflags);
}

int valid_cpu_freq_stat;

/* get the cpu freq counting and return the cpu energy since the last call of the function */
// int cpu_freq_stat_time_in_state(resources_stats *info) {
// 	/* read the system cpu energy based on given max- and min- cpu energy, and frequencies statistics */
// 	FILE *fp;
// 	char line[32] = {'\0'};
// 	int i, max_i;
// 	struct dirent *dirent;
// 	char *cpu_freq_file = NULL;
// 	float energy_total;
// 	unsigned long long tmp;
// 	unsigned long long freqs[16];
// 	const char filenametest[]="/sys/devices/system/cpu/cpu0/cpufreq/stats/time_in_state";
// 	energy_total = 0.0;
// 	info->sys_cpu_energy = energy_total;// TODO we need a default value in case can not collect data !!!
// 	if(valid_cpu_freq_stat==0)
// 		return FAILURE;
// 	/* check if system support cpu freq counting */
// 	if(access( filenametest, F_OK ) != -1 ) {
// 		// file exists
// 	} else {
// 		// file doesn't exist
// 		printf("ERROR: CPU frequency statistics are not supported.\n");
// 		valid_cpu_freq_stat=0;
// 		return FAILURE;
// 	}
// 	float power_range = parameters_value[0] - parameters_value[1];
// 	DIR *dir = opendir("/sys/devices/system/cpu");
// 	if(!dir) {
// 		printf("ERROR: Could not open directory /sys/devices/system/cpu\n");
// 		valid_cpu_freq_stat=0;
// 		return FAILURE;
// 	}
// 	while ((dirent = readdir(dir))) {
// 		/* for each entry name starting by cpuxx */
// 		if(strncmp(dirent->d_name,"cpu", 3) != 0)
// 			continue;
// 		cpu_freq_file=concat_and_free(&cpu_freq_file, "/sys/devices/system/cpu/");
// 		cpu_freq_file=concat_and_free(&cpu_freq_file, dirent->d_name);
// 		cpu_freq_file=concat_and_free(&cpu_freq_file, "/cpufreq/stats/time_in_state");
// 		fp = fopen(cpu_freq_file, "r");
// 		cpu_freq_file[0]='\0';//in the next loop the string will be empty
// 		//memset(cpu_freq_file, '\0', malloc_usable_size(cpu_freq_file));
// 		if(!fp)
// 			continue;
// 		for (i = 0; !feof(fp) && (i <= 15); i++) {
// 			if(fgets(line, 32, fp) == NULL)
// 				break;
// 			sscanf(line, "%llu %llu", &tmp, &freqs[i]);
// 			/* each line has a pair like "<frequency> <time>", which means this CPU spent <time> usertime at <frequency>.
// 			unit of <time> is 10ms*/
// 		}
// 		max_i = i - 1;
// 		fclose(fp);
// 		for (i = 0; i <= max_i; i++) //for each possible freq value
// 			energy_total += (parameters_value[0] - power_range * i / max_i) * freqs[i] * 10.0; // in milliJoule
// 	}
// 	if(cpu_freq_file!= NULL) free(cpu_freq_file);
// 	closedir(dir);
// 	info->sys_cpu_energy = energy_total;
// 	printf("energy_total %.6f\n",energy_total);
// 	return SUCCESS;
// }//cpu_freq_stat_time_in_state


//equation of the line:   Energy = (Energy_max- Energy_min)/(Freq_max - Freq_min)

//The energy consumed is proportional to the cube of the frequency
//Calculation of the function Energy = k*Freq^3 + c
// (Energy_2 - Energy_1) = k (Freq_2^3 - Freq_1^3) ==> k = (Energy_2 -Energ_1)/(Freq_2^3 - Freq_1^3)
// c = Energy_2 - k * Freq_2^ 3

/* get the cpu freq counting and return the cpu energy since the last call of the function */
int cpu_freq_stat(resources_stats *info, energy_model param_energy ) {
	/* read the system cpu energy based on given max- and min- cpu energy, and frequencies statistics */
	FILE *fp;
	char line[32] = {'\0'};

// 	int i, max_i;
	struct dirent *dirent;
	char *cpu_freq_file = NULL;
	unsigned long long freqs;
	const char filenametest[]="/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
	float energy_each, energy_total = 0.0;	
// 	unsigned long long tmp;
// 	unsigned long long freqs_states[16];
	float temp_freq;
	info->sys_cpu_energy = energy_total;// TODO we need a default value in case can not collect data !!!
	/* check if system support cpu freq counting */
	if(access( filenametest, F_OK ) != -1 ) {
		// file exists
	} else {
		// file doesn't exist
		printf("ERROR: CPU frequency statistics are not supported.\n");
		valid_cpu_freq_stat=0;
		return FAILURE;
	}
// 	float power_range = parameters_value[0] - parameters_value[1];//for freq_min and freq_max
	DIR *dir = opendir("/sys/devices/system/cpu");
	if(!dir) {
		printf("ERROR: Could not open directory /sys/devices/system/cpu\n");
		valid_cpu_freq_stat=0;
		return FAILURE;
	}
	int count_cores=0;
	while ((dirent = readdir(dir))) {
		/* for each entry name starting by cpuxx */
		if(strncmp(dirent->d_name,"cpu", 3) != 0)
			continue;
// 		sprintf(cpu_freq_file, "/sys/devices/system/cpu/%s/cpufreq/stats/time_in_state", dirent->d_name);
		cpu_freq_file=concat_and_free(&cpu_freq_file, "/sys/devices/system/cpu/");
		cpu_freq_file=concat_and_free(&cpu_freq_file, dirent->d_name);
// 		cpu_freq_file=concat_and_free(&cpu_freq_file, "/cpufreq/stats/time_in_state");// not appear on my S.O. Ubuntu
		cpu_freq_file=concat_and_free(&cpu_freq_file, "/cpufreq/scaling_cur_freq");
		fp = fopen(cpu_freq_file, "r");
		cpu_freq_file[0]='\0';//in the next loop the filename-string will be empty, but not null
		//memset(cpu_freq_file, '\0', malloc_usable_size(cpu_freq_file));
		if(!fp)
			continue;
// 		for (i = 0; !feof(fp) && (i <= 15); i++) {// it was used for the time_in_state
			if(fgets(line, 32, fp) == NULL)
				break;
			sscanf(line, "%llu", &freqs );//units are in KHz at scaling_cur_freq
// 			sscanf(line, "%llu %llu", &tmp, &freqs_states[i]);// it was used for the time_in_state
			/* each line has a pair like "<frequency> <time>", which means this CPU spent <time> usertime at <frequency>.
			unit of <time> is 10ms*/
// 		}
// 		max_i = i - 1;//that is the num of states// it was used for the time_in_state
		fclose(fp);
// 		for (i = 0; i <= max_i; i++) {// it was used for the time_in_state
// 			energy_each = (parameters_value[0] - power_range * i / max_i) * freqs_states[i] * 10.0; // in milliJoule// it was used for the time_in_state
// 			energy_total += energy_each;// it was used for the time_in_state
// 		}
		temp_freq=((float)freqs )/1000000.0;
		energy_total += param_energy.cpu_factor_c + param_energy.cpu_factor_k * temp_freq*temp_freq*temp_freq ; // in milliJoule
		count_cores++;
	}
	if(count_cores>0) energy_total = energy_total  / count_cores;
	printf("freq %.2f  energy %.2f\n",temp_freq,energy_total);
	if(cpu_freq_file!= NULL) free(cpu_freq_file);
	closedir(dir);
	info->sys_cpu_energy = energy_total;//in watts
// 	printf("energy_total %.6f\n",energy_total);
	return SUCCESS;
}

/* read the perf counter from the file descriptor */
unsigned long long read_perf_counter(int fd) {
	unsigned long long single_count[3];
	size_t res;
	if(fd <= 0)
		return 0;
	res = read(fd, single_count, 3 * sizeof(unsigned long long));
	if(res == 3 * sizeof(unsigned long long)) {
		return single_count[0];
	}
	return 0;
}

/** read from /proc filesystem all statistics;
* get the cpu energy consumption based on cpu freq statistics;
* read from perf counter the hardware cache misses 
* returns a set of bits indicating if error on the different functions
* returns 0 if completed successfully all the functions
*/
int read_and_check(int fd, int pid, resources_stats *info, energy_model param_energy ) {
	//pending to replace with: int linux_resources_sample(const int pid, struct resources_stats_t *stat_after ) ;
	int value_to_return=0;
	if(CPU_stat_process_read(pid, info) != SUCCESS)
		value_to_return+=1;

	if(io_stats_read(pid, info) != SUCCESS)
		value_to_return+=2;

	float ticksPerSecond=sysconf(_SC_CLK_TCK); // ticks per sec
	if(CPU_stat_read(info,ticksPerSecond) != SUCCESS)
		value_to_return+=4;

	if(valid_cpu_freq_stat!=0)
		if(cpu_freq_stat(info, param_energy ) != SUCCESS)
			value_to_return+=8; 

	info->before_pid_accum_l2_cache_misses = info->pid_accum_l2_cache_misses;
	if(fd<=0){
		info->pid_accum_l2_cache_misses = 0;
	}else{
		info->pid_accum_l2_cache_misses = read_perf_counter(fd);
	}
// 	if(info->pid_accum_l2_cache_misses < info->before_pid_accum_l2_cache_misses) value_to_return+=16;
	info->pid_l2_cache_misses= info->pid_accum_l2_cache_misses - info->before_pid_accum_l2_cache_misses;
	return value_to_return;
}
// int oldread_and_check(int fd, int pid, resources_stats *info) {
// 	printf("**** read_and_check  \n\n");
// 	int value_to_return=0;
// 	if(read_pid_time(pid, info) <= 0)
// 		value_to_return+=1;
// 
// 	if(read_pid_io(pid, info) <=0)
// 		value_to_return+=2;
// 
// 	if(read_sys_time(info) <= 0)
// 		value_to_return+=4;
// 	
// 	if(cpu_freq_stat(info) <= 0)
// 		value_to_return+=8;
// 
// 	info->pid_l2_cache_misses = read_perf_counter(fd);
// 	if(info->pid_l2_cache_misses <= 0)
// 		value_to_return+=16;
// 	return value_to_return;
// }
 


// /* read the process runtime from /proc/[pid]/stat */
// int read_pid_time(int pid, resources_stats *info) {
// 	FILE *fp;
// 	char *line= (char *) malloc(1024);
// 	char pid_cpu_file[128] = {'\0'};
// 	char tmp_str[32];
// 	char tmp_char;
// 	unsigned long long tmp, pid_utime, pid_stime;
// 	if(line == NULL) {
// 		printf("ERROR: Could not alocate memory\n");
// 		return 0;
// 	}
// 	sprintf(pid_cpu_file, "/proc/%d/stat", pid);
// 	fp = fopen(pid_cpu_file, "r");
// 	if(fp == NULL) {
// 		printf("ERROR: Could not open file %s\n", pid_cpu_file);
// 		return 0;
// 	}
// 	if(fgets(line, 1024, fp) != NULL) {
// 		sscanf(line, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu",
// 			(int *)&tmp, tmp_str, &tmp_char, (int *)&tmp, (int *)&tmp, (int *)&tmp, (int *)&tmp, (int *)&tmp,
// 			(unsigned int *)&tmp, (unsigned long *)&tmp, (unsigned long *)&tmp, (unsigned long *)&tmp, 
// 			(unsigned long *)&tmp, (unsigned long *)&pid_utime, (unsigned long *)&pid_stime);
// 	}
// 	info->pid_runtime = pid_utime + pid_stime;
// 	fclose(fp);
// 	if(line!=NULL) free(line);
// 	return 1;
// }
// 
// /* read the process read_bytes, write_bytes, and cancelled_writes from /proc/[pid]/io */
// int read_pid_io(int pid, resources_stats *info) {
// 	FILE *fp;
// 	char line[128];
// 	char disk_file[128] = {'\0'};
// 	sprintf(disk_file, "/proc/%d/io", pid);
// 	fp = fopen(disk_file, "r");
// 	if(fp == NULL) {
// 		printf("ERROR: Could not open file %s\n", disk_file);
// 		return 0;
// 	}
// 	while (fgets(line, 128, fp) != NULL) {
// 		if(!strncmp(line, "read_bytes:", 11)) {
// 			sscanf(line + 12, "%llu", &info->pid_read_bytes);
// 		} else if(!strncmp(line, "write_bytes:", 12)) {
// 			sscanf(line + 13, "%llu", &info->pid_write_bytes);
// 		} else if(!strncmp(line, "cancelled_write_bytes:", 22)) {
// 			sscanf(line + 23, "%llu", &info->pid_cancelled_writes);
// 		}
// 	}
// 	fclose(fp);
// 	return 1;
// }
// 
// /* read the system itv and runtime from /proc/stat*/
// int read_sys_time(resources_stats *info) {
// 	FILE *fp;
// 	char *line= (char *) malloc(1024);
// 	const char cpu_file[] = "/proc/stat";
// 	unsigned long long cpu_user, cpu_nice, cpu_sys, cpu_idle, cpu_iowait, cpu_hardirq, cpu_softirq, cpu_steal;
// 	if(line == NULL) {
// 		printf("ERROR: Could not alocate memory\n");
// 		return 0;
// 	}
// 	fp = fopen(cpu_file, "r");
// 	if(fp == NULL) {
// 		printf("ERROR: Could not open file %s\n", cpu_file);
// 		return 0;
// 	}
// 	if(fgets(line, 1024, fp) != NULL) {
// 		sscanf(line+5, "%llu %llu %llu %llu %llu %llu %llu %llu",
// 			&cpu_user,
// 			&cpu_nice,
// 			&cpu_sys,
// 			&cpu_idle,
// 			&cpu_iowait,
// 			&cpu_hardirq,
// 			&cpu_softirq,
// 			&cpu_steal);
// 	}
// 	info->sys_itv = cpu_user + cpu_nice + cpu_sys + cpu_idle + cpu_iowait + cpu_hardirq + cpu_softirq + cpu_steal;
// 	info->sys_runtime = cpu_user + cpu_sys;
// 	fclose(fp);
// 	if(line!=NULL) free(line);
// 	return 1;
// }



int power_monitor(int pid, char *DataPath, long sampling_interval) {
	int jj;
	for (jj=0;jj<9;jj++){
		parameters_value[jj]=1.0;
	}

	energy_model param_energy ;

	param_energy.cpu_factor_c = 15.2; // watts on idle state
	param_energy.cpu_factor_k = 3.8;  //= (Energy_2 -Energ_1)/(Freq_2^3 - Freq_1^3) = (90- 40)/(2^3-1^3)=14  <-- freq in GHz
	param_energy.L2CACHE_LINE_SIZE=128;
	param_energy.L2CACHE_MISS_LATENCY=59.80;
	param_energy.MEMORY_POWER=2.016;
	param_energy.sata_drive=15.0;
	param_energy.case_fan= 1;
	param_energy.hd_power = 8;
	param_energy.motherboard_power = 40;

	param_energy.E_DISK_R_PER_KB=0.0556;
	param_energy.E_DISK_W_PER_KB=0.0438;
	param_energy.E_NET_SND_PER_KB=0.14256387;
	param_energy.E_NET_RCV_PER_KB=0.24133936;
	
	valid_cpu_freq_stat=1;
	/*create and open the file*/
	printf("start power monitor\n");
	char *FileName=NULL;
	FileName=concat_and_free(&FileName, DataPath);
	FileName=concat_and_free(&FileName, "/");
	FileName=concat_and_free(&FileName, METRIC_NAME_3);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if(fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		if(FileName !=NULL) free(FileName);
		return FAILURE;
	}
	struct timespec timestamp_before, timestamp_after;
	double timestamp_ms;
	float duration, sys_cpu_power, pid_cpu_power, pid_mem_power, pid_disk_power;
 	resources_stats delta;
	int fd_perf = create_perf_stat_counter(pid);
	if(fd_perf <= 0){
		printf("ERROR: create_perf_stat_counter: %s\n", FileName);
// 		if(fp!=NULL) fclose(fp); fp=NULL;
// 		if(FileName !=NULL) free(FileName);
// 		return FAILURE;
	}
	int returned_value =read_and_check(fd_perf, pid, &delta, param_energy );
	if(returned_value != SUCCESS){
		printf("ERROR: 3: %s\n", FileName);
// 		if(fd_perf!=NULL) fclose(fd_perf); fd_perf=NULL;
// 		if(FileName !=NULL) free(FileName);
// 		return FAILURE;
	}
	/*in a loop do data sampling and write into the file*/
	while(running) {
		/*get before timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp_before);
		usleep(sampling_interval * 1000);
		int returned_value =read_and_check(fd_perf, pid, &delta, param_energy);
		if(returned_value != SUCCESS){
			printf("ERROR: 4: %s\n", FileName);
// 			if(fp!=NULL) fclose(fp); fp=NULL;
// 			if(FileName !=NULL) free(FileName);
// 			return FAILURE;
		}
// 		printf(" delta->sys_runtime %llu  \n",delta.sys_runtime);
		delta.throughput = (delta.read_bytes + delta.write_bytes) / sampling_interval; //in bytes/s, must be after linux_resources_sample or read_and_check

		/*get after timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp_after);
// 		/*calculate the increments of counters; update the values before and after */
// 		if(calculate_and_update(&before, &after, &delta) <= 0)// needed??
// 			continue;
		/* calculate the time interval in seconds */
		duration = timestamp_after.tv_sec - timestamp_before.tv_sec + ((timestamp_after.tv_nsec - timestamp_before.tv_nsec) / 1.0e9);
		/* system-wide cpu power in milliwatt */
		sys_cpu_power = (delta.sys_cpu_energy * delta.sys_runtime) / (delta.sys_itv * duration);
		/* pid-based cpu power in milliwatt */
		pid_cpu_power = (sys_cpu_power * delta.pid_runtime) / delta.sys_runtime;

// 		printf("delta.sys_runtime %.6f\n",delta.sys_runtime);
// 		printf("delta->sys_itv %llu  \n",delta.sys_itv);
// 		printf("duration %.5f  \n",duration);
// 		printf("sys_cpu_power %.6f\n",sys_cpu_power);
// 		printf("delta.sys_cpu_energy %.6f\n",delta.sys_cpu_energy);

		/* pid-based memory access power in milliwatt */
		pid_mem_power = ((delta.read_bytes + delta.write_bytes - delta.cancelled_writes) / param_energy.L2CACHE_LINE_SIZE + delta.pid_l2_cache_misses) *
					param_energy.L2CACHE_MISS_LATENCY * param_energy.MEMORY_POWER * 1.0e-6 / duration;
		/* pid-based disk access power in milliwatt */
		pid_disk_power = (delta.read_bytes * param_energy.E_DISK_R_PER_KB + (delta.write_bytes - delta.cancelled_writes) * param_energy.E_DISK_W_PER_KB) / 
						(1024 * duration);
		timestamp_ms = timestamp_after.tv_sec * 1000.0  + (double)(timestamp_after.tv_nsec / 1.0e6);
		if(fp==NULL){
			printf("Error file handler not valid\n");
			exit (1);
		}else{
			fprintf(fp, "\"local_timestamp\":\"%.1f\", \"%s\":%.3f, \"%s\":%.3f, \"%s\":%.3f, \"%s\":%.3f\n", timestamp_ms,
				"total_CPU_power", sys_cpu_power,
				"process_CPU_power", pid_cpu_power,
				"process_mem_power", pid_mem_power,
				"process_disk_power", pid_disk_power);
		}
	}
	fclose(fp);
	if(FileName !=NULL) free(FileName);
	return SUCCESS;
}
