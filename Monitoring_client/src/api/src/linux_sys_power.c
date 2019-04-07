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

// 1 Watt = 1 Joule/ 1 second.
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
#include <stdbool.h>
#define SUCCESS 0
#define FAILURE 1

int valid_cpu_freq_stat;

// size_t execute_command(const char *command, char *comout, size_t *comalloc){
//	// Setup our pipe for reading and execute our command.
//	FILE *fd;
//	size_t comlen = 0;
//	fd = popen(command,"r");//if (!fd) == if(fp==NULL) This if statement implicitly checks "is not 0", so we reverse that to mean "is 0".
//	if (fd!=NULL){
//		char buffer[256];
//		size_t chread;
//		/* Use fread so binary data is dealt with correctly */
//		while ((chread = fread(buffer, 1, sizeof(buffer), fd)) != 0) {
//			if (comlen + chread +1>= *comalloc) {
//				*comalloc = *comalloc + *comalloc;
//				comout = (char *) realloc(comout, *comalloc * sizeof(char));
//			}
//			memmove(comout + comlen, buffer, chread);// destination source numbytes
//			comlen += chread;
//		}
//		pclose(fd);
//	}
//	comout[comlen]='\0';
//	/* We can now work with the output as we please. Just print out to confirm output is as expected */
//	//fwrite(comout, 1, comlen, stdout);
//	return comlen;
// }


int int_pow(const int number, const unsigned int power){
	unsigned int result =1;
	for(int i=0;i<power;i++)
		result *= number;
	return result;
}




unsigned int lengthstring(const char input[]){
	unsigned int i=0;
	while(input[i]!='\0') i++;
	return i;
}

void to_lowercase(char *output_str, const char *input_str){
	int i=0;
	while(input_str[i]!='\0'){
		if((input_str[i]>64)&&(input_str[i]<91)) {
			output_str[i]=input_str[i]+ 97 - 65;
		}else{
			output_str[i]=input_str[i];
		}
		i++;
	}
	output_str[i]='\0';
}

int int_digits(const int value){
	unsigned int i=0;
	int n;
	n=value;
	while(n>0){
		n=n/10;
		i++;
	}
	return i;
}

void int_to_string(char *output, const int value, const int digits){
	unsigned int i;
	unsigned int contador=digits-1;
	for(i=0;i<digits;i++){
		output[i]=48+((int)((value)/int_pow(10, contador) )%10);
		contador--;
	}
	output[digits]='\0';
}

/*
* digits: the number of digits in the string:
* number= 5, digits =2 --> 02
* number= 5, digits =3 --> 002
*/
void add_int_to_str(char *str_2_add, const int position, const int number, const int digits){
	char minumero[20];
	int_to_string(minumero, number, digits);
	for(int i=0;i<=digits;i++) str_2_add[position+i]=minumero[i];//included the end of string char '\0'	
}

/*
* copy from position "start" of the string "input" into the string "output"
* until find end of string, end of line or a space character ' '
*/
// unsigned int process_str(char *input, char *output, unsigned const int start, const unsigned int max_output_size){
//	unsigned int j=0;
//	while((input[j+start]!='\n')&&(input[j+start]!='\0')&&(input[j+start]!=' ')&&(j+1<max_output_size) ){
//		output[j]=input[j+start];
//		j++;
//	}
//	output[j]='\0';
//	return (start+j);
// }

/*
* copy from position "start" of the string "input" into the string "output"
* until find end of string or end of line 
*/
// unsigned int getline_str(char *input, char *output, unsigned const int start){
//	unsigned int j=0;
//	while((input[j+start]!='\n')&&(input[j+start]!='\0')){
//		output[j]=input[j+start];
//		j++;
//	}
//	output[j]='\0';
//	return (start+j+1);
// }

//search for the string cad1 in the string source, starting from position "start"
//if found: return ths position where starts the cad1 in the string source
//if not found: returns a not valid value, which is the lenght of source +1 
// unsigned int find_str(int start, char source[], const char cad1[]){
//	int longsource=lengthstring(source);
//	int longcad=lengthstring(cad1);
//	if (longsource<longcad)
//		return(longsource+1);
//	int i=start;
//	int iguales;
//	do{
//		iguales=true;	
//		for (int j=0;j<longcad;j++)
//			if (source[i+j]!=cad1[j])
//				iguales=false;
//		i++;
//	} while ((iguales==false) && (i<=(longsource-longcad)));
//	if (iguales==false)
//		return(longsource+1);
//	i--;// Habiamos sumado uno de mas
//	return (i);
// }

//removes the string "cadenaBuscar" from the string "source"
//returns true if the string "cadenaBuscar" was found and removed
// int remove_str(int start ,char source[], const char cadenaBuscar[]){
//	int longsource=lengthstring(source);
//	int longcad=lengthstring(cadenaBuscar);
//	if (longsource>=longcad){
//		int i=find_str(start,source, cadenaBuscar);
//		if (i+longcad<=longsource) { // Encontramos la cadena 1 y empieza en i1
//			for (int j=i;j<=longsource;j++) 
//				source[j]=source[j+longcad];
//			return(true);
//		//}else{
//		//	printf(" i %i longcad %i long source %i\n",i,longcad,longsource);
//		}
//	}
//	return(false);
// }

//it searches for the first integer from the position "start" in a text string
//if found number:
//		the funtion returns TRUE if found number
//		returns in "end" the ending position of the number found in the string.
//		returns dato with the value of the number found
// if not found number:
//		the funtion returns FALSE if found number
//		returns in "end" the position of the end of the line, or end of the string if not found any end of line
//		returns dato with zero
int get_first_int(const int start,int *end, char *loadstr, int *dato){
	int pos=start;
	int found=false;
	*dato=0;
	while( ((loadstr[pos]<48)||(loadstr[pos]>57)) && (loadstr[pos]!='\n') && (loadstr[pos]!='\0') ){
		pos++;
	}
	if((loadstr[pos]>=48)&&(loadstr[pos]<=57)){
		found=true;
		while((loadstr[pos]>=48)&&(loadstr[pos]<=57)){
			*dato=*dato*10+loadstr[pos]-48;
			pos++;
		}
	}	
	if ((loadstr[pos]=='\n') || (loadstr[pos]=='\0')) pos++;
	*end=pos;
	return found;
}

//if can remove the label from the beggining of string
//then returns the int remaining in the string
int find_int_from_label(char *loadstr, const char *label, int *to_update){
	char result[200];
	if(remove_str(0,loadstr, label)==true){
		int i=0;
		while( (i<lengthstring(loadstr)) && ( ((loadstr[i]>47)&&(loadstr[i]<58)) || (loadstr[i]==' ') )){
			result[i]=loadstr[i];
			i++;
		}
		result[i]='\0';
		*to_update=atoi(result);
		return true;
	}
	return false;
}

int find_float_from_label(char *loadstr, const char *label, float *to_update){
	char result[200];
	if(remove_str(0,loadstr, label)==true){
		int i=0;
		while( (i<lengthstring(loadstr)) && ( ((loadstr[i]>47)&&(loadstr[i]<58)) || (loadstr[i]==' ') || (loadstr[i]=='.') )){
			result[i]=loadstr[i];
			i++;
		}
		result[i]='\0';
		*to_update=atof(result);
		return true;
	}
	return false;
}

int find_lint_from_label(char *loadstr, const char *label, long int *to_update){
	char result[200];
	if(remove_str(0,loadstr, label)==true){
		int i=0;
		while( (i<lengthstring(loadstr)) && ( ((loadstr[i]>47)&&(loadstr[i]<58)) || (loadstr[i]==' ') )){
			result[i]=loadstr[i];
			i++;
		}
		result[i]='\0';
		*to_update=atol(result);
		return true;
	}
	return false;
}
 


int longitud(const char cadena1[]){
	int i=0;
	while (cadena1[i]!='\0') 
		i++;
	return(i);
}


int buscacadena (int ini,char *LINEA,const char *cad1){
	int iguales,j;
	int i=ini;
	if (longitud(LINEA)<longitud(cad1)) 
		return(longitud(LINEA));
	do {
		iguales=true;
		for (j=0;j<longitud(cad1);j++) {
			if (LINEA[i+j]!=cad1[j]) 
				iguales=false;
		}
		i++;
	}
	while ((iguales==false) && (i<=(longitud(LINEA)-longitud(cad1))));
	if (iguales==false) 
		return(longitud(LINEA)+1);
	i--;// Habiamos sumado uno de mas
	return (i);
}


//returns in the string temp , the text between cad1 y cad2 found in string LINEA starting from position ini
int textoentre(const char cad1[],const char cad2[], int ini, char *LINEA, char temp[]) {
	int i1,i2,j;
	stpcpy (temp, "");
	if (longitud(LINEA)>longitud(cad1)+longitud(cad2)) {
		i1=buscacadena(ini,LINEA, cad1);
		if (i1<=(longitud(LINEA)-longitud(cad1))) { // Encontramos la cadena 1 y empieza en i1
			i1=i1+longitud(cad1);
			if (cad2[0]!='\0') {
				if ((longitud(LINEA)-i1)>longitud(cad2)) {
					i2=buscacadena(i1,LINEA, cad2);//empezamos a partir de la primera cadena
					if (i2<=(longitud(LINEA)-longitud(cad2))) {// Encontramos la cadena 2 y empieza en i2
						for (j=0;j<i2-i1;j++)
							temp[j]=LINEA[i1+j];
						temp[i2-i1]='\0';
						return 1;
					}
				}
			} else {
				i2=longitud(LINEA);
				for (j=0;j<i2-i1;j++)
					temp[j]=LINEA[i1+j];
				temp[i2-i1]='\0';
				return 1;
			}
		}
	}
	return 0;
}

 


 
// /* read the process runtime from /proc/[pid]/stat */
// int read_pid_time(int pid, resources_stats *info) {
//	FILE *fp;
//	char *line= (char *) malloc(1024);
//	char pid_cpu_file[128] = {'\0'};
//	char tmp_str[32];
//	char tmp_char;
//	unsigned long long tmp, pid_utime, pid_stime;
//	if(line == NULL) {
//		printf("ERROR: Could not alocate memory\n");
//		return 0;
//	}
//	sprintf(pid_cpu_file, "/proc/%d/stat", pid);
//	fp = fopen(pid_cpu_file, "r");
//	if(fp == NULL) {
//		printf("ERROR: Could not open file %s\n", pid_cpu_file);
//		return 0;
//	}
//	if(fgets(line, 1024, fp) != NULL) {
//		sscanf(line, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu",
//			(int *)&tmp, tmp_str, &tmp_char, (int *)&tmp, (int *)&tmp, (int *)&tmp, (int *)&tmp, (int *)&tmp,
//			(unsigned int *)&tmp, (unsigned long *)&tmp, (unsigned long *)&tmp, (unsigned long *)&tmp,
//			(unsigned long *)&tmp, (unsigned long *)&pid_utime, (unsigned long *)&pid_stime);
//	}
//	info->pid_runtime = pid_utime + pid_stime;
//	fclose(fp);
//	if(line!=NULL) free(line);
//	return 1;
// }
// 
// /* read the process read_bytes, write_bytes, and cancelled_writes from /proc/[pid]/io */
// int read_pid_io(int pid, resources_stats *info) {
//	FILE *fp;
//	char line[128];
//	char disk_file[128] = {'\0'};
//	sprintf(disk_file, "/proc/%d/io", pid);
//	fp = fopen(disk_file, "r");
//	if(fp == NULL) {
//		printf("ERROR: Could not open file %s\n", disk_file);
//		return 0;
//	}
//	while (fgets(line, 128, fp) != NULL) {
//		if(!strncmp(line, "read_bytes:", 11)) {
//			sscanf(line + 12, "%llu", &info->pid_read_bytes);
//		} else if(!strncmp(line, "write_bytes:", 12)) {
//			sscanf(line + 13, "%llu", &info->pid_write_bytes);
//		} else if(!strncmp(line, "cancelled_write_bytes:", 22)) {
//			sscanf(line + 23, "%llu", &info->pid_cancelled_writes);
//		}
//	}
//	fclose(fp);
//	return 1;
// }

// /* read the system itv and runtime from /proc/stat*/
// int read_sys_time(resources_stats *info) {
//	FILE *fp;
//	char *line= (char *) malloc(1024);
//	const char cpu_file[] = "/proc/stat";
//	unsigned long long cpu_user, cpu_nice, cpu_sys, cpu_idle, cpu_iowait, cpu_hardirq, cpu_softirq, cpu_steal;
//	if(line == NULL) {
//		printf("ERROR: Could not alocate memory\n");
//		return 0;
//	}
//	fp = fopen(cpu_file, "r");
//	if(fp == NULL) {
//		printf("ERROR: Could not open file %s\n", cpu_file);
//		return 0;
//	}
//	if(fgets(line, 1024, fp) != NULL) {
//		sscanf(line+5, "%llu %llu %llu %llu %llu %llu %llu %llu",
//			&cpu_user,
//			&cpu_nice,
//			&cpu_sys,
//			&cpu_idle,
//			&cpu_iowait,
//			&cpu_hardirq,
//			&cpu_softirq,
//			&cpu_steal);
//	}
//	info->sys_itv = cpu_user + cpu_nice + cpu_sys + cpu_idle + cpu_iowait + cpu_hardirq + cpu_softirq + cpu_steal;
//	info->sys_runtime = cpu_user + cpu_sys;
//	fclose(fp);
//	if(line!=NULL) free(line);
//	return 1;
// }


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
	// pid > 0 and cpu == -1 measures the specified process/thread on any CPU.
	//pid == -1 and cpu >= 0 measures all processes/threads on the specified CPU.
	//               last one requires CAP_SYS_ADMIN capability or a
	//               last one: /proc/sys/kernel/perf_event_paranoid value of less than 1.
	const int mcpu = -1;
	const int mgroup_fd = -1;
	const unsigned int mflags=0;
	return syscall(__NR_perf_event_open, &attr, pid, mcpu, mgroup_fd, mflags);
	
// #ifdef USERSPACE_ONLY
//	attr.exclude_kernel = 1;
//	attr.exclude_hv = 1;
//	attr.exclude_idle = 1;
//	attr.exclude_callchain_kernel = 1;
// #endif
//	attr.type = PERF_TYPE_HARDWARE;
//	attr.config = PERF_COUNT_HW_CPU_CYCLES;
//	cycles_fd = sys_perf_event_open(&attr, 0, -1, -1, 0);
//	if (cycles_fd < 0) {
//		perror("sys_perf_event_open");
//		exit(1);
//	}
//	/* We use cycles_fd as the group leader in order to ensure
//	 * both counters run at the same time and our CPI statistics are
//	 * valid. */
//	attr.disabled = 0; /* The group leader will start/stop us */
//	attr.type = PERF_TYPE_HARDWARE;
//	attr.config = PERF_COUNT_HW_INSTRUCTIONS;
//	instructions_fd = sys_perf_event_open(&attr, 0, -1, cycles_fd, 0);
//	if (instructions_fd < 0) {
//		perror("sys_perf_event_open");
//		exit(1);
//	}
}


/* get the cpu freq counting and return the cpu energy since the last call of the function */
// int cpu_freq_stat_time_in_state(resources_stats *info) {
//	/* read the system cpu energy based on given max- and min- cpu energy, and frequencies statistics */
//	FILE *fp;
//	char line[32] = {'\0'};
//	int i, max_i;
//	struct dirent *dirent;
//	char *cpu_freq_file = NULL;
//	float energy_total;
//	unsigned long long tmp;
//	unsigned long long freqs[16];
//	const char filenametest[]="/sys/devices/system/cpu/cpu0/cpufreq/stats/time_in_state";
//	energy_total = 0.0;
//	info->sys_cpu_energy = energy_total;// TODO we need a default value in case can not collect data !!!
//	if(valid_cpu_freq_stat==0)
//		return FAILURE;
//	/* check if system support cpu freq counting */
//	if(access( filenametest, F_OK ) != -1 ) {
//		// file exists
//	} else {
//		// file doesn't exist
//		printf("ERROR: CPU frequency statistics are not supported.\n");
//		valid_cpu_freq_stat=0;
//		return FAILURE;
//	}
//	float power_range = parameters_value[0] - parameters_value[1];
//	DIR *dir = opendir("/sys/devices/system/cpu");
//	if(!dir) {
//		printf("ERROR: Could not open directory /sys/devices/system/cpu\n");
//		valid_cpu_freq_stat=0;
//		return FAILURE;
//	}
//	while ((dirent = readdir(dir))) {
//		/* for each entry name starting by cpuxx */
//		if(strncmp(dirent->d_name,"cpu", 3) != 0)
//			continue;
//		cpu_freq_file=concat_and_free(&cpu_freq_file, "/sys/devices/system/cpu/");
//		cpu_freq_file=concat_and_free(&cpu_freq_file, dirent->d_name);
//		cpu_freq_file=concat_and_free(&cpu_freq_file, "/cpufreq/stats/time_in_state");
//		fp = fopen(cpu_freq_file, "r");
//		cpu_freq_file[0]='\0';//in the next loop the string will be empty
//		//memset(cpu_freq_file, '\0', malloc_usable_size(cpu_freq_file));
//		if(!fp)
//			continue;
//		for (i = 0; !feof(fp) && (i <= 15); i++) {
//			if(fgets(line, 32, fp) == NULL)
//				break;
//			sscanf(line, "%llu %llu", &tmp, &freqs[i]);
//			/* each line has a pair like "<frequency> <time>", which means this CPU spent <time> usertime at <frequency>.
//			unit of <time> is 10ms*/
//		}
//		max_i = i - 1;
//		fclose(fp);
//		for (i = 0; i <= max_i; i++) //for each possible freq value
//			energy_total += (parameters_value[0] - power_range * i / max_i) * freqs[i] * 10.0; // in milliJoule
//	}
//	if(cpu_freq_file!= NULL) free(cpu_freq_file);
//	closedir(dir);
//	info->sys_cpu_energy = energy_total;
//	printf("energy_total %.6f\n",energy_total);
//	return SUCCESS;
//}//cpu_freq_stat_time_in_state


//equation of the line: Energy = (Energy_max- Energy_min)/(Freq_max - Freq_min)

//The energy consumed is proportional to the cube of the frequency
//Calculation of the function Energy = k*Freq^3 + c
// (Energy_2 - Energy_1) = k (Freq_2^3 - Freq_1^3) ==> k = (Energy_2 -Energ_1)/(Freq_2^3 - Freq_1^3)
// c = Energy_2 - k * Freq_2^ 3

/* get the cpu freq counting and return the cpu energy since the last call of the function */
int cpu_freq_stat(resources_stats *info, energy_model param_energy) {
	/* read the system cpu energy based on given max- and min- cpu energy, and frequencies statistics */
	FILE *fp;
	char line[32] = {'\0'};

//	int i, max_i;
	struct dirent *dirent;
	char *cpu_freq_file = NULL;
	unsigned long long freqs;
	const char filenametest[]="/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
	float energy_total = 0.0; 
//	unsigned long long tmp;
//	unsigned long long freqs_states[16];
//	long int temp_freq;
//	info->sys_cpu_energy = energy_total;// TODO we need a default value in case can not collect data !!!
	/* check if system support cpu freq counting */
	if(access( filenametest, F_OK ) != -1 ) {
		// file exists
	} else {
		// file doesn't exist
		printf("Warning: CPU frequency statistics are not supported in this machine, maybe it is a Virtual Machine.\n");
		valid_cpu_freq_stat=0;
		return FAILURE;
	}

	float power_range = param_energy.MAX_CPU_POWER - param_energy.MIN_CPU_POWER;//for freq_min and freq_max
	DIR *dir = opendir("/sys/devices/system/cpu");
	if(!dir) {
		printf("ERROR: Could not open directory /sys/devices/system/cpu\n");
//		valid_cpu_freq_stat=0;
		return FAILURE;
	}
	int count_cores=0;
	while ((dirent = readdir(dir))) {
		/* for each entry name starting by cpuxx */
		if(strncmp(dirent->d_name,"cpu", 3) != 0)
			continue;
//		sprintf(cpu_freq_file, "/sys/devices/system/cpu/%s/cpufreq/stats/time_in_state", dirent->d_name);
		cpu_freq_file=concat_and_free(&cpu_freq_file, "/sys/devices/system/cpu/");
		cpu_freq_file=concat_and_free(&cpu_freq_file, dirent->d_name);
//		cpu_freq_file=concat_and_free(&cpu_freq_file, "/cpufreq/stats/time_in_state");// not appear on my S.O. Ubuntu
		cpu_freq_file=concat_and_free(&cpu_freq_file, "/cpufreq/scaling_cur_freq");
		fp = fopen(cpu_freq_file, "r");
		cpu_freq_file[0]='\0';//in the next loop the filename-string will be empty, but not null
		//memset(cpu_freq_file, '\0', malloc_usable_size(cpu_freq_file));
		if(!fp)
			continue;
//		for (i = 0; !feof(fp) && (i <= 15); i++) {// it was used for the time_in_state
			if(fgets(line, 32, fp) == NULL)
				break;
			sscanf(line, "%llu", &freqs);//units are in KHz at scaling_cur_freq
//			sscanf(line, "%llu %llu", &tmp, &freqs_states[i]);// it was used for the time_in_state
			/* each line has a pair like "<frequency> <time>", which means this CPU spent <time> usertime at <frequency>.
			unit of <time> is 10ms*/
//		}
//		max_i = i - 1;//that is the num of states// it was used for the time_in_state
		fclose(fp);
//		for (i = 0; i <= max_i; i++) // it was used for the time_in_state
//			energy_total += (parameters_value[0] - power_range * i / max_i) * freqs_states[i] * 10.0; // in milliJoule// it was used for the time_in_state
//		temp_freq=((float)freqs )/1000000.0;
//		energy_total += param_energy.cpu_factor_c + param_energy.cpu_factor_k * temp_freq*temp_freq*temp_freq; // in milliJoule
		energy_total += param_energy.MIN_CPU_POWER + power_range * (freqs-param_energy.freq_min)/(param_energy.freq_max-param_energy.freq_min); // in milliJoule
//		printf("  core num %i,  freq %llu Energy %.6f \n",count_cores, freqs, param_energy.MIN_CPU_POWER + power_range * (freqs-param_energy.freq_min)/(param_energy.freq_max-param_energy.freq_min));
//		printf("   ---> %.6f\n", energy_total);
		count_cores++;
	}
//	printf("-----\ncount_cores %i\n",count_cores);
//	printf("  param_energy.MIN_CPU_POWER %.3f\n",param_energy.MIN_CPU_POWER);
//	printf("  param_energy.MAX_CPU_POWER %.3f\n",param_energy.MAX_CPU_POWER);
//	printf("  freq_max %llu freq_min %llu\n",param_energy.freq_max, param_energy.freq_min);
	
//	printf("CPU energy_total  =%.6f Joules\n",energy_total);
	if(cpu_freq_file!= NULL) free(cpu_freq_file);
	closedir(dir);
	info->sys_cpu_energy = energy_total;//in watts
//	printf("energy_total %.6f\n",energy_total);
	return SUCCESS;
}

/* read the perf counter from the file descriptor */
unsigned long long read_perf_counter(int fd) {
	unsigned long long single_count[3];
	size_t res;
	if(fd <= 0)
		return 0;
	res = read(fd, single_count, 3 * sizeof(unsigned long long));
	if(res == 3 * sizeof(unsigned long long))
		return single_count[0];
	return 0;
}

/** read from /proc filesystem all statistics;
* get the cpu energy consumption based on cpu freq statistics;
* read from perf counter the hardware cache misses 
* returns a set of bits indicating if error on the different functions
* returns 0 if completed successfully all the functions
*/
int read_and_check(int fd, int pid, resources_stats *info, energy_model param_energy ) {
	//pending to replace with: int linux_resources_sample(const int pid, struct resources_stats_t *stat_after );
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
//	if(info->pid_accum_l2_cache_misses < info->before_pid_accum_l2_cache_misses) value_to_return+=16;
	info->pid_l2_cache_misses= info->pid_accum_l2_cache_misses - info->before_pid_accum_l2_cache_misses;
	return value_to_return;
}
// int oldread_and_check(int fd, int pid, resources_stats *info) {
//	printf("**** read_and_check\n\n");
//	int value_to_return=0;
//	if(read_pid_time(pid, info) <= 0)
//		value_to_return+=1;
// 
//	if(read_pid_io(pid, info) <=0)
//		value_to_return+=2;
// 
//	if(read_sys_time(info) <= 0)
//		value_to_return+=4;
//	
//	if(cpu_freq_stat(info) <= 0)
//		value_to_return+=8;
// 
//	info->pid_l2_cache_misses = read_perf_counter(fd);
//	if(info->pid_l2_cache_misses <= 0)
//		value_to_return+=16;
//	return value_to_return;
// }

/* check if values are increasing; calculate the differences in the time interval; update the before values with after values */
int calculate_and_update(resources_stats *before, resources_stats *after, resources_stats *delta) {
	if(after->sys_itv <= before->sys_itv)
		return 0;
	if(after->sys_runtime <= before->sys_runtime)
		return 0;
	delta->sys_itv = after->sys_itv - before->sys_itv;
	delta->sys_runtime = after->sys_runtime - before->sys_runtime;
	delta->pid_runtime = after->pid_runtime - before->pid_runtime;
	delta->read_bytes = after->read_bytes - before->read_bytes;
	delta->write_bytes = after->write_bytes - before->write_bytes;
	delta->cancelled_writes = after->cancelled_writes - before->cancelled_writes;
	delta->pid_l2_cache_misses = after->pid_l2_cache_misses - before->pid_l2_cache_misses;
	delta->sys_cpu_energy = after->sys_cpu_energy - before->sys_cpu_energy;
	memcpy(before, after, sizeof(resources_stats));
	return 1;
}

unsigned int numcores(char *comout, size_t *comalloc) {
	const char command[]= "grep -c ^processor /proc/cpuinfo;";
	execute_command(command, comout, comalloc);
	return atoi(comout);
}


#define NET_STAT_FILE "/proc/%d/net/dev"

/** @brief Gets current network stats (send and receive bytes)
 * collected metrics on nets_info->rcv_bytes and nets_info->send_bytes
 @return SUCCESS or otherwise FAILURE*/
// int procesa_network_stat_read(int pid,  struct task_data_t *my_task_data) {
int procesa_network_stat_read(char *comout, size_t *comalloc, struct task_data_t *my_task_data) {
	FILE *fp;
	unsigned int temp;
	unsigned long long temp_rcv_bytes, temp_send_bytes;
	int subtask=0;
	for(subtask=0;subtask< my_task_data->totaltid;subtask++){
		if(my_task_data->subtask[subtask]->updated==true){
			if(my_task_data->subtask[subtask]->pstid==my_task_data->subtask[subtask]->pspid){
//				int pstid=my_task_data->subtask[subtask]->pstid;
				int pspid=my_task_data->subtask[subtask]->pspid;
				if(pspid!=0){
				/* values reset to zeros */
				my_task_data->subtask[subtask]->rcv_bytes = 0;
				my_task_data->subtask[subtask]->send_bytes = 0;
				char pid_net_file[128], line[1024];
				sprintf(pid_net_file, NET_STAT_FILE, pspid);
				if ((fp = fopen(pid_net_file, "r")) != NULL) {
//					printf( "Error: Cannot open %s.\n", pid_net_file);
//					return false;
				while(fgets(line, 1024, fp) != NULL) {
					char *sub_line_eth = strstr(line, "eth");
					if (sub_line_eth != NULL) {
						sscanf(sub_line_eth + 5, "%llu%u%u%u%u%u%u%u%llu",
							&temp_rcv_bytes, &temp, &temp, &temp, &temp, &temp, &temp, &temp,
							&temp_send_bytes);
						my_task_data->subtask[subtask]->rcv_bytes += temp_rcv_bytes;
						my_task_data->subtask[subtask]->send_bytes += temp_send_bytes;
					}
					char *sub_line_wlan = strstr(line, "wlan");
					if (sub_line_wlan != NULL) {
						sscanf(sub_line_wlan + 6, "%llu%u%u%u%u%u%u%u%llu",
							&temp_rcv_bytes, &temp, &temp, &temp, &temp, &temp, &temp, &temp,
							&temp_send_bytes);
						my_task_data->subtask[subtask]->rcv_bytes += temp_rcv_bytes;
						my_task_data->subtask[subtask]->send_bytes += temp_send_bytes;
					}
					char *sub_line_lo= strstr(line, "lo");
					if (sub_line_lo != NULL) {
						sscanf(sub_line_lo + 6, "%llu%u%u%u%u%u%u%u%llu",
							&temp_rcv_bytes, &temp, &temp, &temp, &temp, &temp, &temp, &temp,
							&temp_send_bytes);
						my_task_data->subtask[subtask]->rcv_bytes += temp_rcv_bytes;
						my_task_data->subtask[subtask]->send_bytes += temp_send_bytes;
					}
				}
				fclose(fp);
				}
				}
			}
		}//end if
	}
	return true;
}

// cat /proc/cpuinfo | grep "MHz"
//example
// cpu MHz		: 400.040
// cpu MHz		: 400.027
// cpu MHz		: 400.020
// cpu MHz		: 400.008
void procesa_cpuinfo( char *comout, size_t *comalloc, unsigned int argmaxcores, struct task_data_t *my_task_data ) {
	const char command[]= "if [ -e /proc/cpuinfo ];then cat /proc/cpuinfo | grep \"MHz\";fi;";//expected to update every 10ms
	unsigned int i,contador,maxcores=argmaxcores;
	const int size_loadstr=250;
	char loadstr[size_loadstr];
	float my_temp_float;
//	long long int actual_time=mycurrenttime();
	size_t comlen = execute_command( command, comout, comalloc);
	//update all the loads to 0 of all tasks registerd in my_task_data
	//first need to find if the pspid and pstid were already registered in my_task_data, its position will be stored in "index"
	//if not find register, then create a new entry and define the current time as start time
	if(comlen!=0){
		i=0;
		contador=0;
		while((comout[i]!='\0')&&(i<comlen)){
			if(contador>= my_task_data->maxcores){
				printf(" counter of freqs exceds maxcores %i\n",my_task_data->maxcores);
				return;
			}
			while (comout[i]==' ')
				i++;
			int end_string=getline_str(comout, loadstr, i);// i<end_string ->> (comout[i]!='\n')
			i=end_string;
			find_float_from_label( loadstr, "cpu MHz		:", &my_temp_float);
			my_task_data->cores[contador].core_freq=(long int)(my_temp_float*1000);//in KHz
			contador++;//count of cores
		}
	}
}

int power_monitor(int pid, char *DataPath, long sampling_interval, long long int start_app_time, struct app_report_t *my_app_report, struct task_data_t *my_task_data_a) {
	int i;
	size_t comalloc = 8256;
	char *comout = (char *) malloc(comalloc * sizeof(char));
	unsigned int maxcores= numcores(comout, &comalloc);
	my_task_data_a->maxprocesses =30;
	my_task_data_a->maxcores=30;
	my_task_data_a->subtask = (struct sub_task_data **) malloc( my_task_data_a->maxprocesses * sizeof(struct sub_task_data *));
	for(i=0;i<my_task_data_a->maxprocesses;i++){
		my_task_data_a->subtask[i] = (struct sub_task_data *) malloc(sizeof(struct sub_task_data ));
		my_task_data_a->subtask[i]->rcv_bytes = 0;
		my_task_data_a->subtask[i]->send_bytes = 0;
	}
	my_task_data_a->cores = (struct cores_data *) malloc( my_task_data_a->maxcores * sizeof(struct cores_data));
	my_task_data_a->totaltid=0;
	my_task_data_a->pid=pid;
	my_task_data_a->maxtotaltid=1;
	my_task_data_a->first_start=0;
	my_task_data_a->last_end=0;
	for(i=0;i<my_task_data_a->maxprocesses;i++)
		my_task_data_a->subtask[i]->totaltime=0;

	for(i=0;i<my_task_data_a->maxcores;i++){
		my_task_data_a->cores[i].total_joules_core=0;
		my_task_data_a->cores[i].time_of_last_measured=0;
		my_task_data_a->cores[i].total_watts_core=0;
	}
	energy_model param_energy;
	param_energy.freq_min=400000;
	param_energy.freq_max=2800000;

	param_energy.MAX_CPU_POWER=55.5/4.0;//[0] <--- per core
	param_energy.MIN_CPU_POWER=11.0/4.0;//[1] <--- per core
	param_energy.L2CACHE_LINE_SIZE=128;//[4]
	param_energy.L2CACHE_MISS_LATENCY=59.80;//[3]
	param_energy.MEMORY_POWER=2.016;//[2]
	param_energy.case_fan= 1;
	param_energy.motherboard_power = 40;

	param_energy.sata_drive=15.0;

	param_energy.hd_power = 8;
//	param_energy.E_DISK_R_PER_MB=0.0556;//[5]
//	param_energy.E_DISK_W_PER_MB=0.0438;//[6]
	// Read/Write  6.00 Watts
	// Idle        5.50 Watts
	// Standby     0.80 Watts
	// Sleep       0.80 Watts

// values from https://www.tomshardware.co.uk/desktop-hdd.15-st4000dm000-4tb,review-32729-6.html
//	WD RED WD30EFRX 3TB 5400 rpm  5.4W
//	Seagate Desktop HDD 15  4TB 5900 rpm 5.9W
//	Hitachi Deskstar 5K4000 4TB 5400 rpm 6.0 W
//	Hitachi Deskstar 5K3000 3TB 5400 rpm 6.4 W
//	WD Caviar Green WD30ERZRX 3TB 5400 rpm 7.0W
//	Seagate Barracuda 3TB 7200 rpm 7.9 W
//	Hitachi Deskstar 7K3000 3TB 7200 rom 8.6W
//	Hitachi Deskstar 7K4000 4TB 7200 rpm 8.7W
//	WD Black WD4001FAEX 4TB 7200 rpm 9.3 W
//	Seagate Barracuda XT 3TB 7200 rpm 9.5W

	param_energy.E_NET_SND_PER_MB=0.14256387;
	param_energy.E_NET_RCV_PER_MB=0.24133936;

// char parameters_name[9][32] = {"MAX_CPU_POWER", "MIN_CPU_POWER",// fields [0] [1]
//	"MEMORY_POWER", "L2CACHE_MISS_LATENCY", "L2CACHE_LINE_SIZE",//fields [2] [3] [4]
//	"E_DISK_R_PER_MB", "E_DISK_W_PER_MB", //fields [5] [6]
//	"E_NET_SND_PER_KB", "E_NET_RCV_PER_KB"};
// float parameters_value[9];

	valid_cpu_freq_stat=1;
	/*create and open the file*/
	printf("start power monitor\n");
	char *FileName=NULL;
	FileName=concat_and_free(&FileName, DataPath);
	FileName=concat_and_free(&FileName, "/"); //kk
	FileName=concat_and_free(&FileName, METRIC_NAME_3);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if(fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		if(FileName !=NULL) free(FileName);
		return FAILURE;
	}
	struct timespec  timestamp_after;//timestamp_before,
	double timestamp_ms;

	resources_stats before;//, delta;, after
	
	before.min_write_bytes = 0;
	before.max_write_bytes = 0;
	before.min_read_bytes = 0;
	before.max_read_bytes = 0;
	before.write_bytes = 0;
	before.read_bytes = 0;
	before.accum_read_bytes = 0;
	before.before_accum_read_bytes = 0;
	before.accum_write_bytes = 0;
	before.before_accum_write_bytes = 0;
	before.accum_cancelled_writes = 0;
	before.before_accum_cancelled_writes = 0;
	
	
	
	 
	
	
	int fd_perf = create_perf_stat_counter(pid);
	if(fd_perf <= 0){
		printf("WARNING: create_perf_stat_counter is not supported.\n");
//		if(fp!=NULL) fclose(fp); fp=NULL;
// 		if(FileName !=NULL) free(FileName);
// 		return FAILURE;
	}
// 	int returned_value =
	read_and_check(fd_perf, pid, &before, param_energy);
// 	if(returned_value != SUCCESS){
// 		printf("ERROR: 3: %s\n", FileName); 
// 		if(FileName !=NULL) free(FileName);
// 		return FAILURE;
// 	}
	int my_totaltid=0;
	if(my_totaltid!=my_task_data_a->totaltid){
	for(i=my_totaltid;i<my_task_data_a->totaltid;i++){
		my_task_data_a->subtask[i]->time_of_last_measured=0;
		my_task_data_a->subtask[i]->start_comp=0;
	}
	my_totaltid=my_task_data_a->totaltid;	
	}
	
	/*in a loop do data sampling and write into the file*/
	while(running) {
		if(my_totaltid!=my_task_data_a->totaltid){
		for(i=my_totaltid;i<my_task_data_a->totaltid;i++){
			my_task_data_a->subtask[i]->time_of_last_measured=0;
			my_task_data_a->subtask[i]->start_comp=0;
		}
		my_totaltid=my_task_data_a->totaltid;	
		}
		
		
		procesa_cpuinfo( comout, &comalloc, maxcores, my_task_data_a);

		maxcores=	procesa_pid_load(pid, maxcores, my_task_data_a, param_energy);
		procesa_task_io(my_task_data_a);
		procesa_network_stat_read(comout, &comalloc, my_task_data_a);

//		procesa_system_mem(comout, &comalloc, &mysystem);//global of the system
//		mysystem.cpu_system_load= procesa_system_load(comout, &comalloc, array_valores);
//		totaldevices=procesa_disk_stats( comout, &comalloc, newdata, olddata, &valid_disk_data);

		/*get before timestamp in ms*/
//		clock_gettime(CLOCK_REALTIME, &timestamp_before);
		usleep(sampling_interval * 1000);
//		int returned_value =read_and_check(fd_perf, pid, &after, param_energy);
//		if(returned_value != SUCCESS){
//			printf("ERROR: 4: %s\n", FileName);
//			if(fp!=NULL) fclose(fp); fp=NULL;
//			if(FileName !=NULL) free(FileName);
//			return FAILURE;
//		}
//		printf(" delta->sys_runtime %llu\n",delta.sys_runtime);
//		delta.throughput = (delta.read_bytes + delta.write_bytes) / sampling_interval; //in bytes/s, must be after linux_resources_sample or read_and_check

		/*get after timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp_after);
		/*calculate the increments of counters; update the values before and after */
//		if(calculate_and_update(&before, &after, &delta) <= 0)// needed??
//			continue;
//		calculate_and_update(&before, &after, &delta);
		/* calculate the time interval in seconds */
//		duration = timestamp_after.tv_sec - timestamp_before.tv_sec + ((timestamp_after.tv_nsec - timestamp_before.tv_nsec) / 1.0e9);
		/* system-wide cpu power in milliwatt */
//		sys_cpu_power = (delta.sys_cpu_energy * delta.sys_runtime) / (delta.sys_itv * duration);
		/* pid-based cpu power in milliwatt */
//		pid_cpu_power = (sys_cpu_power * delta.pid_runtime) / delta.sys_runtime;

//		printf("delta.sys_runtime %.6f\n",delta.sys_runtime);
//		printf("delta->sys_itv %llu\n",delta.sys_itv);
//		printf("duration %.5f\n",duration);
//		printf("sys_cpu_power %.6f\n",sys_cpu_power);
//		printf("delta.sys_cpu_energy %.6f\n",delta.sys_cpu_energy);

		/* pid-based memory access power in milliwatt */
//		pid_mem_power = ((delta.read_bytes + delta.write_bytes - delta.cancelled_writes) / param_energy.L2CACHE_LINE_SIZE + delta.pid_l2_cache_misses) *
//					param_energy.L2CACHE_MISS_LATENCY * param_energy.MEMORY_POWER * 1.0e-6 / duration;
		/* pid-based disk access power in milliwatt */
//		pid_disk_power = 8;//(delta.read_bytes * param_energy.E_DISK_R_PER_MB + (delta.write_bytes - delta.cancelled_writes) * param_energy.E_DISK_W_PER_MB) / (1024 * duration);

		timestamp_ms = timestamp_after.tv_sec * 1000.0 + (double)(timestamp_after.tv_nsec / 1.0e6);
		if(fp==NULL){
			printf("Error file handler not valid\n");
			exit (1);
		}else{
//			fprintf(fp, "\"local_timestamp\":\"%.1f\", \"%s\":%.3f, \"%s\":%.3f, \"%s\":%.3f, \"%s\":%.3f\n", timestamp_ms,
//				"total_CPU_power", sys_cpu_power,
//				"process_CPU_power", pid_cpu_power,
//				"process_mem_power", pid_mem_power,
//				"process_disk_power", pid_disk_power);

		long long int actual_time;
//		if ( my_task_data_a->last_end!=0){
//			actual_time = my_task_data_a->last_end;
//		}else{
			actual_time=mycurrenttime();
//		}
		my_app_report->read_bytes = 0;//kk
		my_app_report->write_bytes = 0;
		my_app_report->cancelled_writes = 0;
		my_app_report->pid_net_power = 0.0;
		my_app_report->total_cpu_energy=0.0;
		for(i=0;i<my_task_data_a->totaltid;i++){
//			printf(" AAAA  \"cpu_load\":\"%.2f\"\n", my_task_data_a->subtask[i]->pcpu);
			my_app_report->read_bytes += my_task_data_a->subtask[i]->rchar;
			my_app_report->write_bytes += my_task_data_a->subtask[i]->wchar;
			my_app_report->read_bytes += my_task_data_a->subtask[i]->read_bytes;
			my_app_report->write_bytes += my_task_data_a->subtask[i]->write_bytes;
//			my_task_data_a->subtask[i]->syscr,
//			my_task_data_a->subtask[i]->syscw,
//			my_task_data_a->subtask[i]->pmem
			my_app_report->cancelled_writes += my_task_data_a->subtask[i]->cancelled_write_bytes;
			my_app_report->pid_net_power += (param_energy.E_NET_RCV_PER_MB*my_task_data_a->subtask[i]->rcv_bytes + param_energy.E_NET_SND_PER_MB* my_task_data_a->subtask[i]->send_bytes)* 1.0e-6;
			
			if(my_task_data_a->subtask[i]->time_of_last_measured==0){
				my_task_data_a->subtask[i]->time_of_last_measured=actual_time;
				my_task_data_a->subtask[i]->start_comp=actual_time;
			}
				float total_time = ((1.0e-9)* (actual_time - my_task_data_a->subtask[i]->time_of_last_measured));
				
//			if (i==1)
//			printf(" BBBBa  \"cpu_energy [%i]\":\"%.2f\" [ %.6f * %.6f * %.6f= %.6f ] %.2f \n", i, my_task_data_a->subtask[i]->total_cpu_energy,
//				   (1.0e-9)*(my_task_data_a->subtask[i]->time_of_last_measured   -  my_task_data_a->subtask[i]->start_comp),
//				   0.01*my_task_data_a->subtask[i]->pcpu, 
//					param_energy.MIN_CPU_POWER,
//	
//		  			   (1.0e-9)*(my_task_data_a->subtask[i]->time_of_last_measured -  my_task_data_a->subtask[i]->start_comp)*
//				   0.01*my_task_data_a->subtask[i]->pcpu*
//					param_energy.MIN_CPU_POWER,
//		  
//					total_time
//					);
			my_task_data_a->subtask[i]->total_cpu_energy+=0.01* my_task_data_a->subtask[i]->pcpu*param_energy.MIN_CPU_POWER*total_time;//kk
			my_task_data_a->subtask[i]->time_of_last_measured=actual_time;//kk
			my_app_report->total_cpu_energy+= my_task_data_a->subtask[i]->total_cpu_energy;//kk
		}
// 		my_app_report->total_cpu_energy =0;
// 		for(int c=0;c<my_task_data_a->maxcores;c++)
// 			my_app_report->total_cpu_energy+=my_task_data_a->cores[c].total_watts_core;
		my_app_report->pid_l2_cache_misses=0;//kk
		my_app_report->pid_mem_power = ((my_app_report->read_bytes + my_app_report->write_bytes - my_app_report->cancelled_writes) / param_energy.L2CACHE_LINE_SIZE + my_app_report->pid_l2_cache_misses) * param_energy.L2CACHE_MISS_LATENCY * param_energy.MEMORY_POWER* 1.0e-9;// / duration;
		my_app_report->pid_disk_power = param_energy.hd_power;
		my_app_report->total_hd_energy=0.0;
//		if (my_task_data_a->first_start!=0)
			my_app_report->total_hd_energy=my_app_report->pid_disk_power*(actual_time - start_app_time)/(1.0e9);//kk
		my_app_report->total_watts= my_app_report->total_cpu_energy + my_app_report->pid_mem_power;//kk
// 		+ my_app_report->total_hd_energy
// 		+ my_app_report->pid_net_power;
		fprintf(fp, "\"local_timestamp\":\"%.1f\",",timestamp_ms);
		
		
		
// 		fprintf(fp, " \"start_comp\":\"%lli\",",i,my_task_data_a->subtask[i]->start_comp);
// 		fprintf(fp, "\"actual_time\":\"%lli\",",actual_time);
// 		fprintf(fp, "\"time_of_last_measured\":\"%lli\",",my_task_data_a->subtask[i]->time_of_last_measured);
		
		fprintf(fp, "\"cpu_power\":\"%5.3f\",",my_app_report->total_cpu_energy);
// 		fprintf(fp, "\"io_power\":\"%5.3f\",",my_app_report->total_hd_energy); //last_end and first start are in ns
		fprintf(fp, "\"mem_power\":\"%5.3f\",",my_app_report->pid_mem_power);
// 		fprintf(fp, "\"net_power\":\"%5.3f\",",my_app_report->pid_net_power);
		fprintf(fp, "\"total_watts\":\"%5.3f\",",my_app_report->total_watts);
		fprintf(fp, "\"cost_power\":\"%5.6f\"\n",my_app_report->total_watts*0.25/(1000.0*3600.0));
//		fprintf(fp, " pidpower %.3f J",my_app_report->pid_disk_power);
//		fprintf(fp, " time %.3f s\n", (actual_time - start_app_time)/(1.0e9));
		}
	}//end while running
	fclose(fp);
	printf(" end power pid=%i\n",pid);
	if(FileName !=NULL) free(FileName);
//	for(i=0;i<my_task_data_a.maxprocesses;i++)
//		free(my_task_data_a.subtask[i]);
//	free(my_task_data_a.cores);
//	free(my_task_data_a.subtask);
	free(comout);
	return SUCCESS;
}
