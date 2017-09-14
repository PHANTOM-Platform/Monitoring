#include <unistd.h>
#include <sys/stat.h>
//#include <cstring> //for c++
#include <string.h>
#include "mf_api.h"
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "monitoring_support.h"


/*
Get the pid, and setup the DataPath for data storage 
*/
static int api_prepare_loc(char *Data_path){
	/*reset Data_path*/
	size_t size ;//= strlen(Data_path);
	Data_path[0]='\0'; //memset(Data_path, '\0', size);
	/*get the pid*/
	int pid = getpid();
	/*get the pwd*/
	char buf_1[256] = {'\0'};
	int ret = readlink("/proc/self/exe", buf_1, 200);
	if(ret == -1) {
		printf("readlink /proc/self/exe failed.\n");
		exit(0);
	}
	int pos_last_slash=0;
	/* extract path folder of executable from it's path */
	int i=0;
	while(buf_1[i]!='\0'){
		if(buf_1[i]=='/') pos_last_slash=i;
		Data_path[i]=buf_1[i];
		i++;
	}
	Data_path[pos_last_slash]='\0';
	size = i; //strlen(Data_path);
	/*create the folder with regards of the pid*/
	sprintf(Data_path + size, "/%d", pid);
	struct stat st = { 0 };
	if (stat(Data_path, &st) == -1)
	mkdir(Data_path, 0700);
	return pid;
}

int mf_user_metric_loc(char *user_metrics,  char  *DataPath ){
	int pid=0;
	if(DataPath[0] == '\0') {
		pid = api_prepare_loc(DataPath);
	} 
	/*create and open the file*/
	char FileName[256] = {'\0'};
	sprintf(FileName, "%s/%s", DataPath, "user_defined");
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		return 0;
	}
	struct timespec timestamp;
	/*get current timestamp */
	clock_gettime(CLOCK_REALTIME, &timestamp);
	/*convert to milliseconds */
	double timestamp_ms = timestamp.tv_sec * 1000.0  + (double)(timestamp.tv_nsec / 1.0e6); 
	fprintf(fp, "\"local_timestamp\":\"%.1f\"", timestamp_ms);  
	fprintf(fp, "%s\n",user_metrics);
	/*close the file*/
	fclose(fp);
	return pid;
}

//returns the current time in us
//requires: #include <sys/time.h>
long long unsigned int mycurrenttime (void) { 
	//struct timeval t0 ;	 
	//gettimeofday(&t0, NULL); //https://blog.habets.se/2010/09/gettimeofday-should-never-be-used-to-measure-time.html
	//long long int timeus = (long int) (t0.tv_sec *1000000LL + t0.tv_usec ); 
	struct timespec ts_start; 
	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	long long unsigned int timeus = (long int) (ts_start.tv_sec*1000000000LL +  ts_start.tv_nsec ); 
	return timeus;
}

long long int start_monitoring(char *server, char *regplatformid){
/* MONITORING METRICS */ 
	metrics m_resources;
	m_resources.num_metrics = 2;
	m_resources.local_data_storage = 1; /*remove the file if user unset keep_local_data_flag */
	m_resources.sampling_interval[0] = 1000; // 1s
	strcpy(m_resources.metrics_names[0], "resources_usage");
	m_resources.sampling_interval[1] = 1000; // 1s
	strcpy(m_resources.metrics_names[1], "disk_io");
	m_resources.sampling_interval[2] = 1000; // 1s
	strcpy(m_resources.metrics_names[2], "power"); 
/* MONITORING START */
	//char * DataPath=
		mf_start(server, regplatformid, &m_resources); 
	long long int start_time=mycurrenttime();
	return start_time;
}
 

void prepare_user_metrics( char *currentid, char *DataPath, struct Thread_report single_thread_report ){ 
	long long int total_execution_time = single_thread_report.end_time - single_thread_report.start_time;
printf(" Start time %llu\n", single_thread_report.start_time);
printf(" End time %llu\n", single_thread_report.end_time);

	printf(" TOTAL EXECUTION TIME of thread %s :%9Lu s ", single_thread_report.taskid, (total_execution_time)/1000000000LL);
	long long unsigned int temp_time = (total_execution_time)%1000000000LL;
	printf(" +  %3Lu ms ", temp_time/1000000);
		temp_time = (total_execution_time)%1000000LL;
	printf(" +  %3Lu us ", temp_time/1000);
		temp_time = (total_execution_time)%1000LL;
	printf(" +  %3Lu ns \n", temp_time); 	
/* MONITORING I'd like to store here the duration of each loop --> duration */	
	char metric_value[25] = {'\0'};
	sprintf(metric_value, "%Li", total_execution_time); 
	const char comp_start[]="component_start";
	const char comp_end[]="component_end";
	const char duration_str[]="component_duration"; 
	const char component_name[]="component_name";
	const char run_id[]="runid"; 
	const char ships_txt[]="n_ships_found";
	const char blocks_txt[]="number_of_blocks";
	char user_metrics[356];
	sprintf(user_metrics, ",\"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\",  \"%s\":\"%i\",  \"%s\":\"%u\",    \"%s\":\"%llu\", \"%s\":\"%llu\"", 
		duration_str, metric_value,  // the total execution time of the component
		run_id, currentid,			 // common id of the different components of one execution of the task/workflow/application
		component_name, single_thread_report.taskid,		 // name of the component
		ships_txt, single_thread_report.n_ships_found,
		blocks_txt, single_thread_report.number_of_blocks, 
		comp_start, single_thread_report.start_time, 	 // start time of the component
		comp_end, single_thread_report.end_time);         // end time of the component

	mf_user_metric_loc(user_metrics,  DataPath );
///* MONITORING I'd like to store here the total nr. of completed loops --> nrLoops */
//	int nrLoops=5;
//	sprintf(metric_value, "%d", nrLoops);
//	char nrLoops_str[]="nrLoops";
//	mf_user_metric_loc(nrLoops_str, metric_value);		 
}

void stop_monitoring(){
/* MONITORING END */
	mf_end();
}
 
void monitoring_send(char *server, char *appid, char *execfile, char *regplatformid, char *DataPath){
	/* MONITORING SEND */
	//char *experiment_id = 
		mf_send(server, appid, execfile, regplatformid);
	//printf("\n> component's experiment_id is %s\n", experiment_id);
}
