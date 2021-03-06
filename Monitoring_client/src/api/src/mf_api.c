/*
* Copyright 2018 High Performance Computing Center, Stuttgart
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
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
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include "mf_util.h"
#include "mf_parser.h"
#include "publisher.h"
#include "linux_resources.h"
#include "xlnx_monitor.h"
#include "linux_sys_power.h"

#ifdef NVML
#if NVML == yes
	#include "nvml_monitor.h"
	#include <nvml.h>
#endif
#endif

#include "mf_api.h"
#include <malloc.h>
#include <math.h>
// #include <sys/time.h>

#define SUCCESS 0
#define FAILED 1

/**
* returns the current time in us
* requires: #include <sys/time.h>
*/
long long int mycurrenttime (void) {
// printf("========================================================\n");
// printf("CLOCK_REALTIME: represents the machine's best-guess as to the current wall-clock, time-of-day time.\n");
// printf(" It can jump forwards and backwards as the system time-of-day clock is changed, including by NTP.\n\n");
// printf("========================================================\n");
// printf("CLOCK_MONOTONIC: represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past.\n");
// printf(" It isn't affected by changes in the system time-of-day clock.\n");
// printf(" If you want to compute the elapsed time between two events observed on the one machine without an intervening reboot, this is the best option.\n");
	//struct timeval t0 ;
	//gettimeofday(&t0, NULL); //https://blog.habets.se/2010/09/gettimeofday-should-never-be-used-to-measure-time.html
	//long long int timeus = (long int) (t0.tv_sec *1000000LL + t0.tv_usec);
	struct timespec ts_start;
	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	//needed if next look some embedded devices like the Odroid XU4,
	// Odroid XU4 cpu is not tls compliant, replace "CLOCK_MONOTONIC" with "CLOCK_REALTIME"
    while (ts_start.tv_nsec > 1.0e9) {
        ts_start.tv_sec=ts_start.tv_sec+1;
        ts_start.tv_nsec= ts_start.tv_nsec- 1.0e9;
    }
	long long int timeus = (long long int) (ts_start.tv_sec*1000000000LL + ts_start.tv_nsec);
	return timeus;
}

char *mycurrenttime_str (void) {
	long long int mytime = mycurrenttime();
	char *new_string = (char *) malloc(60);
	new_string=llitoa(mytime,new_string);
	return new_string;
}

/*******************************************************************************
* Variable Declarations
******************************************************************************/
int running;
int keep_local_data_flag = 1;
char parameters_name[9][32] = {"MAX_CPU_POWER", "MIN_CPU_POWER",
	"MEMORY_POWER", "L2CACHE_MISS_LATENCY", "L2CACHE_LINE_SIZE",
	"E_DISK_R_PER_KB", "E_DISK_W_PER_KB",
	"E_NET_SND_PER_KB", "E_NET_RCV_PER_KB"};
// float parameters_value[9];

char DataPath[256];
pthread_t threads[MAX_NUM_METRICS];
unsigned int num_threads;
unsigned int pid=0;//considered as undefined if equal zero

FILE *logFile=NULL;

/*******************************************************************************
* Forward Declarations
******************************************************************************/
static int api_prepare(char *Data_path);
static void *MonitorStart(void *arg);
int get_config_parameters(const char *server,const char *platform_id,const char *token);

/*******************************************************************************
* Function Definitions
******************************************************************************/

struct app_report_t *reserve_app_report(const unsigned int num_of_threads, const char *currentid){
	struct app_report_t *my_reservation = (struct app_report_t *)malloc(2*sizeof(struct app_report_t));
	my_reservation->num_of_threads=num_of_threads;
	strcpy(my_reservation->currentid,currentid);
	my_reservation->my_thread_report = (struct Thread_report_t **) malloc(num_of_threads * sizeof(struct Thread_report_t *));
	for (unsigned int  i=0;i<num_of_threads;i++){
		my_reservation->my_thread_report[i] = (struct Thread_report_t *) malloc(sizeof(struct Thread_report_t));
		my_reservation->my_thread_report[i]->user_label=NULL;
		my_reservation->my_thread_report[i]->user_value=NULL;
		my_reservation->my_thread_report[i]->metric_time=NULL;
		strcpy(my_reservation->my_thread_report[i]->currentid,currentid);
		my_reservation->my_thread_report[i]->total_metrics=0;
//		my_reservation->my_thread_report[i]->end_time=0;
//		my_reservation->my_thread_report[i]->start_time=0;
	}
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
	/*convert to milliseconds */
	my_reservation->timestamp_ms = timestamp.tv_sec * 1000.0 + (double)(timestamp.tv_nsec / 1.0e6);// in ms + one decimal position
	my_reservation->num_of_processes=1;
	return my_reservation;
}

// int free_app_report(struct app_report_t *my_app_report){
// 	if(my_app_report==NULL) return 0;
// 	for (int i=0;i<my_app_report->num_of_threads;i++){
// 		if(my_app_report->my_thread_report[i]!=NULL){
// 		for (int j=0;j<my_app_report->my_thread_report[i]->total_metrics;j++){
// 			if(my_app_report->my_thread_report[i]->user_label[j]!=NULL)
// 				free(my_app_report->my_thread_report[i]->user_label[j]);
// 			my_app_report->my_thread_report[i]->user_label[j]=NULL;
// 			if(my_app_report->my_thread_report[i]->user_value[j]!=NULL)
// 				free(my_app_report->my_thread_report[i]->user_value[j]);
// 			my_app_report->my_thread_report[i]->user_value[j]=NULL;
// 			if(my_app_report->my_thread_report[i]->metric_time[j]!=NULL)
// 				free(my_app_report->my_thread_report[i]->metric_time[j]);
// 			my_app_report->my_thread_report[i]->metric_time[j]=NULL;
// 		}
// 		if(my_app_report->my_thread_report[i]->user_label!=NULL)
// 			free(my_app_report->my_thread_report[i]->user_label);
// 		my_app_report->my_thread_report[i]->user_label=NULL;
// 		if(my_app_report->my_thread_report[i]->user_value!=NULL)
// 			free(my_app_report->my_thread_report[i]->user_value);
// 		my_app_report->my_thread_report[i]->user_value=NULL;
// 		if(my_app_report->my_thread_report[i]->metric_time!=NULL)
// 			free(my_app_report->my_thread_report[i]->metric_time);
// 		my_app_report->my_thread_report[i]->metric_time=NULL;
// 		if(my_app_report->my_thread_report[i]!=NULL)
// 			free(my_app_report->my_thread_report[i]);
// 		my_app_report->my_thread_report[i]=NULL;
// 		}
// 	}
// 	if(my_app_report->my_thread_report!=NULL)
// 		free(my_app_report->my_thread_report);
// 	my_app_report->my_thread_report=NULL;
// 	if(my_app_report!=NULL)
// 		free(my_app_report);
// 	my_app_report=NULL;
// 	return 0;
// }

// int reserve_memory_user_def_metrics(struct Thread_report_t *my_thread_report, const unsigned int total_metrics){
//	my_thread_report->total_metrics= total_metrics;
//	my_thread_report->user_label  = (char **) malloc(total_metrics * sizeof(char*));
//	my_thread_report->user_value  = (char **) malloc(total_metrics * sizeof(char*));
//	my_thread_report->metric_time = (char **) malloc(total_metrics * sizeof(char*));
//	for (int i=0;i<total_metrics;i++){
//		my_thread_report->user_label[i]  = (char *) malloc(40 * sizeof(char));
//		my_thread_report->user_value[i]  = (char *) malloc(40 * sizeof(char));
//		my_thread_report->metric_time[i] = (char *) malloc(40 * sizeof(char));
//	}
//	return 0;
// }


int register_user_metric(struct Thread_report_t *my_thread_report, const char *time,const char *label,const int value){
	if(my_thread_report==NULL) return 0;
	int n =my_thread_report->total_metrics;
	if(n==0){
		my_thread_report->user_label  = (char **) malloc( (n+1) * sizeof(char*));
		my_thread_report->user_value  = (char **) malloc( (n+1) * sizeof(char*));
		my_thread_report->metric_time = (char **) malloc( (n+1) * sizeof(char*));
	}else{
		my_thread_report->user_label  = (char **) realloc(my_thread_report->user_label, (n+1)  * sizeof(char*));
		my_thread_report->user_value  = (char **) realloc(my_thread_report->user_value, (n+1)  * sizeof(char*));
		my_thread_report->metric_time = (char **) realloc(my_thread_report->metric_time, (n+1) * sizeof(char*));
	}
	if(my_thread_report->user_label!=NULL){
		my_thread_report->user_label[n]  = (char *) malloc(40 * sizeof(char));
		strcpy(my_thread_report->user_label[n], label); // <<-- LABEL
	}
	if(my_thread_report->user_value!=NULL){
		my_thread_report->user_value[n]  = (char *) malloc(40 * sizeof(char));
		itoa((int) value, my_thread_report->user_value[n]); // <<-- VALUE
	}
	if(my_thread_report->metric_time!=NULL){
		my_thread_report->metric_time[n] = (char *) malloc(40 * sizeof(char));
		my_thread_report->total_metrics=n+1;
		strcpy(my_thread_report->metric_time[n], time); // <<-- TIME
	}
	return 0;
}


struct resources_stats_t *stat_resources=NULL;
struct task_data_t *mmy_task_data_a =NULL;

/**
* Query for a new Execution, return 400 if the execution is not registered yet.
* or 200 in other case.
*
* curl -s -H "Authorization: OAuth ${mytoken}" -H "Content-Type: multipart/form-data" -XPOST -F "UploadJSON=@../web-execmanager/execstatus.json" http://${server}:${execmanager_port}/register_new_exec;
*
* @return NULL if error
*/
char* update_exec(const char *server, const char *filenamepath, const char * token){
	char *URL = NULL;
	char operation[]="POST";
	struct url_data response;
	response.size=0;
	response.data=NULL;
	response.headercode=NULL;
	if(server!=NULL){
		URL=concat_and_free(&URL, server);
	}else{
		printf(" update_exec server is NULL\n");return NULL;
	}
	if(filenamepath==NULL){
		printf(" update_exec filenamepath is NULL\n");return NULL;
	}
	if(token==NULL){
		printf(" update_exec token is NULL\n");return NULL;
	}
	URL=concat_and_free(&URL, "/update_exec");

//	printf(" URL: %s\n", URL);
//	printf(" filenamepath: %s\n", filenamepath);
//	printf(" operation: %s\n", operation);
//	printf(" token: %s\n", token);

	query_message_json_data(URL, NULL, filenamepath, &response, operation, token);

	if(URL!=NULL) free(URL);
	URL=NULL;
	if(response.headercode!=NULL) free(response.headercode);
	response.headercode=NULL;
	if(response.data == NULL) {
		printf("ERROR: Cannot register execution on server %s\n", server);
		return NULL;
	}else if(response.data[0] == '\0') {
		printf("ERROR: Cannot register execution on  server %s\n", server);
		return NULL;
	}
	char* p= response.data;
	return p;
}


char* starting_exec(const char *server, const char *exec_id, const char * token){
	char *URL = NULL;
	struct url_data response;
	response.size=0;
	response.data=NULL;
	response.headercode=NULL;
	if(server!=NULL){
		URL=concat_and_free(&URL, server);
	}else{
		printf(" starting_exec server is NULL\n");
		return NULL;
	}
	if(exec_id==NULL){
		printf(" starting_exec exec_id is NULL\n"); return NULL;
	}
	if(token==NULL){
		printf(" starting_exec token is NULL\n"); return NULL;
	}
	URL=concat_and_free(&URL, "/started_exec?exec_id=\"");
	if(exec_id!=NULL){
		URL=concat_and_free(&URL, exec_id);
	}else{
		printf(" starting_exec exec_id is NULL\n"); return NULL;
	}
	URL=concat_and_free(&URL, "\"");
	char operation[]="POST";
	int result=query_message_json(URL, NULL, NULL, &response, operation, token); //*****
	if(URL!=NULL) free(URL);
	URL=NULL;
	if(response.headercode!=NULL) free(response.headercode);
	response.headercode=NULL;
	if(result!=SUCCESS || response.data==NULL){
		printf("ERROR: Cannot register execution ...\n");
		return NULL;
	}else if(response.data[0] == '\0') {
		printf("ERROR: Cannot register execution\n");
		return NULL;
	}
	char* p= response.data;
	return p;
}

struct Mydate {
	unsigned int name_day, year, month,day, hour, min, sec, msec;
};

void calculate_date(long long int current, struct Mydate *exampledate) {
	if (exampledate==NULL) return;
	int months[12]={31, 28, 31, 30, 31, 30, 31, 31, 30 ,31 ,30 ,31 };
	exampledate->msec =current % 1000;
	current= current /1000;
	exampledate->sec = current %60;
	current =current / 60;
	exampledate->min = current % 60;
	current = current /60;
	exampledate->hour = current %24;
	current = current /24;
	exampledate->year=0;
	exampledate->name_day = current % 7;
	if(current > 365+365+366){
		exampledate->year=3;
		current -= (365+365+366);
	}
	if(current > 1461){
		exampledate->year+=4*(current / 1461);
		current = current % 1461;
	}
	exampledate->year+=current / 365;
	current = current % 365;
	int leap_year= (exampledate->year %4==2) ? 1 : 0;
	if (leap_year==1) months[1]=29;
	exampledate->month=0;
	while(current> months[exampledate->month]){
		current -= months[exampledate->month];
		exampledate->month++;
	}
	exampledate->year= 1970+ exampledate->year;
	exampledate->day= 1+ current;
	exampledate->month=1+exampledate->month;
}

#define FLOAT_TO_LLINT(x) ((x)>=0?(long long int)((x)+0.5):(long long int)((x)-0.5))

char *mf_exec_stats(struct app_report_t my_app_report, const char *application_id, const char *exec_id, const char *platform_id, struct task_data_t *mmy_task_data_a){
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
	param_energy.E_NET_SND_PER_MB=0.14256387;
	param_energy.E_NET_RCV_PER_MB=0.24133936;


	/*create and open the file*/
	char *json_msg = NULL;
	char tempstr[2560] = {'\0'};
	struct Mydate exampledate;
//	double localtimestamp;
	struct timespec timestamp;
	/*get current timestamp */
	clock_gettime(CLOCK_REALTIME, &timestamp);
	/*convert to milliseconds */
	double timestamp_ms = timestamp.tv_sec * 1000.0 + (double)(timestamp.tv_nsec / 1.0e6);
//	concat_and_free(&json_msg, "\"local_timestamp\":\"%.1f\"", timestamp_ms);
	concat_and_free(&json_msg, "{\n\t\"app\":\"");
	if(application_id!=NULL){
		concat_and_free(&json_msg, application_id);
	}else{
		printf(" mf_exec_stats application_id is NULL\n");
	}
	concat_and_free(&json_msg, "\",\n");

//	concat_and_free(&json_msg, "\t\"device\":\"");
//	concat_and_free(&json_msg, platform_id);
//	concat_and_free(&json_msg, "\",\n");

	concat_and_free(&json_msg, "\t\"execution_id\":\"");
	concat_and_free(&json_msg, exec_id);
	concat_and_free(&json_msg, "\",\n");
	concat_and_free(&json_msg, "\t\"start_timestamp\":\"");
	// We convert Epoch into timestamp format, required by ElasticSearch:
	calculate_date(FLOAT_TO_LLINT(my_app_report.timestamp_ms), &exampledate);
	sprintf(tempstr," %u-%02u-%02uT%02u:%02u:%02u.%03u",exampledate.year,exampledate.month,
		exampledate.day, exampledate.hour, exampledate.min, exampledate.sec, exampledate.msec);
	concat_and_free(&json_msg, tempstr);
	concat_and_free(&json_msg, "\",\n");
	concat_and_free(&json_msg, "\t\"end_timestamp\": \"");

	// We convert Epoch into timestamp format, required by ElasticSearch:
	calculate_date(FLOAT_TO_LLINT(timestamp_ms), &exampledate);
	sprintf(tempstr," %u-%02u-%02uT%02u:%02u:%02u.%03u",exampledate.year,exampledate.month,
		exampledate.day, exampledate.hour, exampledate.min, exampledate.sec, exampledate.msec);
	concat_and_free(&json_msg, tempstr);
	concat_and_free(&json_msg, "\",\n");

	concat_and_free(&json_msg, "\t\"start_timestamp_ns\": \"");
	sprintf(tempstr, "%.0f",  1.0e6*my_app_report.timestamp_ms);
	concat_and_free(&json_msg, tempstr);
	concat_and_free(&json_msg, "\",\n");

	concat_and_free(&json_msg, "\t\"end_timestamp_ns\": \"");
	sprintf(tempstr, "%.0f",  1.0e6*timestamp_ms);
	concat_and_free(&json_msg, tempstr);
	concat_and_free(&json_msg, "\",\n");

	concat_and_free(&json_msg, "\t\"total_time_ns\": \"");
//	sprintf(tempstr, "%.3f - %.3f = %.3f", my_app_report.timestamp_ms, timestamp_ms,timestamp_ms -my_app_report.timestamp_ms);
	sprintf(tempstr, "%.0f",  1.0e6*(timestamp_ms -my_app_report.timestamp_ms));
	concat_and_free(&json_msg, tempstr);
	concat_and_free(&json_msg, "\",\n");

	concat_and_free(&json_msg, "\t\"energy\": \"");
	sprintf(tempstr, "%.2f", my_app_report.total_watts);
	concat_and_free(&json_msg, tempstr);
	concat_and_free(&json_msg, "\",\n");

	concat_and_free(&json_msg, "\t\"cpu_power_consumption\": \"");
	sprintf(tempstr, "%.2f", my_app_report.total_cpu_energy);
	concat_and_free(&json_msg, tempstr);
	concat_and_free(&json_msg, "\",\n");

// 	concat_and_free(&json_msg, "\t\"io_power_consumption\": \"");
// 	sprintf(tempstr, "%.2f", my_app_report.total_hd_energy);
// 	concat_and_free(&json_msg, tempstr);
// 	concat_and_free(&json_msg, "\",\n");

	concat_and_free(&json_msg, "\t\"mem_power_consumption\": \"");
	sprintf(tempstr, "%.2f", my_app_report.pid_mem_power);
	concat_and_free(&json_msg, tempstr);
	concat_and_free(&json_msg, "\",\n");

// 	concat_and_free(&json_msg, "\t\"net_power_consumption\": \"");
// 	sprintf(tempstr, "%.2f", my_app_report.pid_net_power);
// 	concat_and_free(&json_msg, tempstr);
// 	concat_and_free(&json_msg, "\",\n");

	concat_and_free(&json_msg, "\t\"num_of_processes\": \"");
	sprintf(tempstr, "%i", my_app_report.num_of_processes);
	concat_and_free(&json_msg, tempstr);
	concat_and_free(&json_msg, "\",\n");
		concat_and_free(&json_msg, "\t\"totaltid\": \"");
		sprintf(tempstr, "%i", mmy_task_data_a->totaltid);
		concat_and_free(&json_msg, tempstr);
		concat_and_free(&json_msg, "\",\n");

		concat_and_free(&json_msg, "\t\"num_of_threads\": \"");
		sprintf(tempstr, "%i", my_app_report.num_of_threads);
		concat_and_free(&json_msg, tempstr);
		concat_and_free(&json_msg, "\",\n");

//	concat_and_free(&json_msg, "\t\"cost_power_consumption\": \"");
//	sprintf(tempstr, "%.2f", my_app_report.total_watts*0.25/(1000.0*3600.0));
//	concat_and_free(&json_msg, tempstr);
//	concat_and_free(&json_msg, "\",\n");

//		printf(" pidpower %.3f J",my_app_report->pid_disk_power);
//		printf(" time %.3f s\n", (actual_time - start_app_time)/(1.0e9) );

	// 2- read the duration_components, and remove the fields
	concat_and_free(&json_msg, "\t\"component_stats\": [\n");
	long long int end_time = mycurrenttime();
		concat_and_free(&json_msg, "\t\t{\n");
		concat_and_free(&json_msg, "\t\t\t\"component_name\": \"");
		concat_and_free(&json_msg, "main_compent");
		concat_and_free(&json_msg, "\",\n");
			concat_and_free(&json_msg, "\t\t\t\"device\":\"");
			concat_and_free(&json_msg, platform_id);
			concat_and_free(&json_msg, "\",\n");
		concat_and_free(&json_msg, "\t\t\t\"component_duration\": \"");
		sprintf(tempstr, "%lli", end_time - my_app_report.start_app);
		concat_and_free(&json_msg, tempstr);
		concat_and_free(&json_msg, "\",\n");
		concat_and_free(&json_msg, "\t\t\t\"component_start\": \"");
		sprintf(tempstr, "%lli", my_app_report.start_app);
		concat_and_free(&json_msg, tempstr);
		concat_and_free(&json_msg, "\",\n");

		concat_and_free(&json_msg, "\t\t\t\"component_end\": \"");
		sprintf(tempstr, "%lli", end_time);
		concat_and_free(&json_msg, tempstr);
		concat_and_free(&json_msg, "\",\n");

		char *json_msgb=save_stats_resources( 1, 3);   //<=======================================================================
		concat_and_free(&json_msg, json_msgb); free(json_msgb);
		concat_and_free(&json_msg, "\n\t\t}\n");

// 	for(int i=0;i<my_app_report.num_of_threads;i++){ //mmy_task_data_a->totaltid;
	for(unsigned int i=0;i<mmy_task_data_a->totaltid;i++){ //;
		concat_and_free(&json_msg, "\t\t,{\n");
// 		printf(" #%i of %i \n",i,mmy_task_data_a->totaltid);fflush(stdout);
		long long int start_time_ns = 0;


// 		if(start_time_ns!=0){
//		if (strcmp(my_app_report.my_thread_report[i]->taskid, "null""){
		long long int end_time_ns = 0;
// 		if(i<my_app_report.num_of_threads){

// 			concat_and_free(&json_msg, "\t\t\t\"component_name\": \"");
// 			concat_and_free(&json_msg, my_app_report.my_thread_report[i]->taskid);
// 			concat_and_free(&json_msg, "\",\n");
// 			printf("   component_name %s pid=%i l_pid=%i l_tid=%i\n",
// 					my_app_report.my_thread_report[i]->taskid,
// 					my_app_report.my_thread_report[i]->pid,
// 					my_app_report.my_thread_report[i]->local_pid,
// 					my_app_report.my_thread_report[i]->local_tid);
// 			fflush(stdout);
	// 		i=mmy_task_data_a->total_user_def
// 		}


		unsigned int m =0;
		while((m< my_app_report.num_of_threads )&& ( mmy_task_data_a->task_def[m]->pstid !=mmy_task_data_a->subtask[i]->pstid )  )
			m++;

		if(m< my_app_report.num_of_threads ){
			start_time_ns =my_app_report.my_thread_report[m]->start_time;
			end_time_ns = my_app_report.my_thread_report[m]->end_time;
			concat_and_free(&json_msg, "\t\t\t\"component_name\": \"");
			concat_and_free(&json_msg, mmy_task_data_a->task_def[m]->component_name);
// 			sprintf(tempstr, "%i %s",mmy_task_data_a->task_def[m]->pstid ,mmy_task_data_a->task_def[m]->component_name );
			concat_and_free(&json_msg, tempstr);
			concat_and_free(&json_msg, "\",\n");
		}else{
			concat_and_free(&json_msg, "\t\t\t\"component_name\": \"unknown\",\n");
		}


// 		concat_and_free(&json_msg, "\t\t\t\"tid taskdefname\": \"");
// 		sprintf(tempstr, "%i %s",mmy_task_data_a->task_def[i]->pstid ,mmy_task_data_a->task_def[i]->component_name );
// 		concat_and_free(&json_msg, tempstr);
// 		concat_and_free(&json_msg, "\",\n");


		concat_and_free(&json_msg, "\t\t\t\"component_duration\": \"");
//		sprintf(tempstr, "%lli", (end_time_ns - start_time_ns)/1000);
		sprintf(tempstr, "%lli", mmy_task_data_a->subtask[i]->time_of_last_measured- mmy_task_data_a->subtask[i]->start_comp);
		concat_and_free(&json_msg, tempstr);
		concat_and_free(&json_msg, "\",\n");


		if((end_time_ns - start_time_ns)>0){
		concat_and_free(&json_msg, "\t\t\t\"component_duration_user_defined_ns\": \"");
		sprintf(tempstr, "%lli", (end_time_ns - start_time_ns));
		concat_and_free(&json_msg, tempstr);
		concat_and_free(&json_msg, "\",\n");
		}

// 			concat_and_free(&json_msg, "\t\t\t\"start_time_ns        \":\"");
// 			sprintf(tempstr, "%lli", start_time_ns);
// 			concat_and_free(&json_msg,tempstr );
// 			concat_and_free(&json_msg, "\",\n");
//
			concat_and_free(&json_msg, "\t\t\t\"start_comp           \":\"");
			sprintf(tempstr, "%lli", mmy_task_data_a->subtask[i]->start_comp);
			concat_and_free(&json_msg,tempstr );
			concat_and_free(&json_msg, "\",\n");

// 			concat_and_free(&json_msg, "\t\t\t\"end_time_ns          \":\"");
// 			sprintf(tempstr, "%lli", my_app_report.my_thread_report[i]->end_time );
// 			concat_and_free(&json_msg,tempstr );
// 			concat_and_free(&json_msg, "\",\n");


			concat_and_free(&json_msg, "\t\t\t\"time_of_last_measured\":\"");
			sprintf(tempstr, "%lli", mmy_task_data_a->subtask[i]->time_of_last_measured);
			concat_and_free(&json_msg,tempstr );
			concat_and_free(&json_msg, "\",\n");

//		concat_and_free(&json_msg, "\t\t\t\"component_start\": \"");
//		sprintf(tempstr, "%.1f", my_app_report.my_thread_report[i]->start_time/1000.0);
//		concat_and_free(&json_msg, tempstr);
//		concat_and_free(&json_msg, "\",\n");
//
//		concat_and_free(&json_msg, "\t\t\t\"component_end\": \"");
//		sprintf(tempstr, "%.1f", my_app_report.my_thread_report[i]->end_time/1000.0);
//		concat_and_free(&json_msg, tempstr);
//		concat_and_free(&json_msg, "\",\n");

			concat_and_free(&json_msg, "\t\t\t\"device\":\"");
			concat_and_free(&json_msg, platform_id);
			concat_and_free(&json_msg, "\",\n");

			concat_and_free(&json_msg, "\t\t\t\"total_cpu_energy_comp\":\"");
			sprintf(tempstr, "%.2f", mmy_task_data_a->subtask[i]->total_cpu_energy);//kk
			concat_and_free(&json_msg,tempstr );
			concat_and_free(&json_msg, "\",\n");

			concat_and_free(&json_msg, "\t\t\t\"mem_power_consumption\": \"");
			float pid_mem_power =   ((mmy_task_data_a->subtask[i]->read_bytes + mmy_task_data_a->subtask[i]->write_bytes - mmy_task_data_a->subtask[i]->cancelled_writes) / param_energy.L2CACHE_LINE_SIZE + 0) * param_energy.L2CACHE_MISS_LATENCY * param_energy.MEMORY_POWER* 1.0e-9;// / duration;
			sprintf(tempstr, "%.2f", pid_mem_power);
			concat_and_free(&json_msg, tempstr);
			concat_and_free(&json_msg, "\",\n");

// 				concat_and_free(&json_msg, "\t\t\t\"net_power_consumption_comp\": \"");
// 				float pid_net_power = (param_energy.E_NET_RCV_PER_MB*mmy_task_data_a->subtask[i]->rcv_bytes + param_energy.E_NET_SND_PER_MB* mmy_task_data_a->subtask[i]->send_bytes)* 1.0e-6;
// 				sprintf(tempstr, "%.2f", pid_net_power);
// 				concat_and_free(&json_msg, tempstr);
// 				concat_and_free(&json_msg, "\",\n");

		concat_and_free(&json_msg, "\t\t\t\"for-debuging-pid-tid\": \"");
		sprintf(tempstr, "%i[%i]%i",mmy_task_data_a->subtask[i]->pspid,i, mmy_task_data_a->subtask[i]->pstid);
		concat_and_free(&json_msg, tempstr);
		concat_and_free(&json_msg, "\",\n");

		concat_and_free(&json_msg, "\t\t\t\"for-debuging-taskid\": \"");
		sprintf(tempstr, "%s",  mmy_task_data_a->subtask[i]->taskid);
		concat_and_free(&json_msg, tempstr);
		concat_and_free(&json_msg, "\",\n");

		// end time of the component
//		char strend_time[100];
//		llint_to_string_alloc(my_app_report.my_thread_report[i]->end_time,strend_time);
//		concat_and_free(&json_msg, "\t\t\t\"component_end\": \"%s\"\n", strend_time);
		char *json_msgc=save_stats_resources_comp( mmy_task_data_a->subtask[i],1,3); // <=======================================================================
		concat_and_free(&json_msg, json_msgc); free(json_msgc);
		concat_and_free(&json_msg, "\n\t\t}");//end of component stats
//		if(i<my_app_report.num_of_threads-1)
//			concat_and_free(&json_msg, ",");//end of component stats
		concat_and_free(&json_msg, "\n");
// 		}

	}
// Notice: "local_timestamp":"1545342592137.9" esta en ms
// Notice: "component_duration":"2057984735" esta en ns
	concat_and_free(&json_msg, "\t]\n");//end of component stats
	concat_and_free(&json_msg, "}\n");
	return json_msg;
}


// 3- send the json to exec_server:port/update_exec
//		url=update_exec
//		var demoreplaceb = document.getElementById("demoreplaceb");
//	var debug_phantom = document.getElementById("debug_phantom");
// //	share_session_storage();
//	if(!sessionStorage.token) {
//		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, try login again, missing token...";
//		if(debug_phantom) debug_phantom.style.display = "block";
//		return false;
//	}
//	if((sessionStorage.token !== undefined) && (sessionStorage.token.length>0)) {
//		var xhr = new XMLHttpRequest();
//		var formData = new FormData();
//		xhr.open('POST', url, true);
//		xhr.setRequestHeader("Authorization", "JWT " + sessionStorage.token);
//		xhr.addEventListener('load', function() {
//			var responseObject = (xhr.responseText);
//			if(demoreplaceb) demoreplaceb.innerHTML = "<pre>"+ responseObject + " status: " +xhr.status+ "</pre>";
//			if(debug_phantom) debug_phantom.style.display = "block";
//		});
//		formData.append("UploadJSON", UploadJSON.files[0]);
// //formData.append("UploadFile", UploadFile.data);
//		xhr.send(formData);//may fault code appear here
//	}else {
//		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, try login again, missing token...";
//		if(debug_phantom) debug_phantom.style.display = "block";
//	}


int mf_user_metric_with_timestamp(char *user_defined_time_stamp, char *metric_name, char *value) {
	if(DataPath[0] == '\0')
		pid = api_prepare(DataPath);
	/*create and open the file*/
	char FileName[256] = {'\0'};
	sprintf(FileName, "%s/%s", DataPath, "user_defined");
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		return 0;
	}
	fprintf(fp, "\"local_timestamp\":\"%s\", \"%s\":%s\n", user_defined_time_stamp, metric_name, value);
	/*close the file*/
	fclose(fp);
	return 1;
}

int mf_user_metric(char *metric_name, char *value) {
	if(DataPath[0] == '\0')
		pid = api_prepare(DataPath);
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
	double timestamp_ms = timestamp.tv_sec * 1000.0 + (double)(timestamp.tv_nsec / 1.0e6);
	fprintf(fp, "\"local_timestamp\":\"%.1f\", \"%s\":\"%s\"\n", timestamp_ms, metric_name, value);
	/*close the file*/
	fclose(fp);
	return 1;
}

static void *Monitor_tid_Start(void *arg) {// increases the totaltid counter,
//	it needs the mmy_task_data_a->subtask[i]->taskid be copied from   my_app_report->my_thread_report[i]->taskid) before call it
	struct task_data_t *my_task_data_a = (struct task_data_t*) arg;
	char FileName[100];
	FILE *fp;
	sprintf(FileName, "%s/%s", DataPath, "resource_usage_comp");
	fp = fopen(FileName, "w");

	while (running==1) {//aqui paco
		stats_sample(my_task_data_a->pid, my_task_data_a);//increases the totaltid counter, pid must be correct !!
		if(my_task_data_a!= NULL){
//			for(int jk=0;jk<my_task_data_a->total_user_def;jk++){ kk
			for(unsigned int jk=0;jk<my_task_data_a->totaltid;jk++){
//			save_stats(fp,my_task_data_a->task_def[jk]->pstid, my_task_data_a);
			save_stats(fp,my_task_data_a->subtask[jk]->pstid, my_task_data_a);   // <=======================================================================
//			print_stats(my_task_data_a->tids[jk], my_task_data_a);
			}
		}
		usleep(my_task_data_a->microsleep);//420000
	}
	fflush(fp);
	fclose(fp);
	return NULL;
}


// void __attribute__((optimize("O0"))) add_tid_to_report(char *component_name, int tid){
void add_tid_to_report(char *component_name, int tid){
	if(mmy_task_data_a!= NULL){
		if(mmy_task_data_a->total_user_def<max_report_tids){
// 			printf(" add_tid_to_report pos %i , tid %i name %s\n", mmy_task_data_a->total_user_def, tid, component_name);
			strcpy(mmy_task_data_a->task_def[mmy_task_data_a->total_user_def]->component_name, component_name);
			mmy_task_data_a->task_def[mmy_task_data_a->total_user_def]->pstid=tid;
			mmy_task_data_a->total_user_def=mmy_task_data_a->total_user_def+1;
		}
	}
}


/**
* Get the pid, and setup the DataPath for data storage.
* For each metric, create a thread, open a file for data storage, and start sampling the metrics periodically.
* @return the path of data files
*/
struct each_metric_t **each_m=NULL;




char* starting_mf_log(const char *server, const int code, const char *msg, const char *user, const char * token){
	char *URL = NULL;
	struct url_data response;
	response.size=0;
	response.data=NULL;
	response.headercode=NULL;
	if(server!=NULL){
		URL=concat_and_free(&URL, server);
	}else{
		printf(" starting_mf_log server is NULL\n");
		return NULL;
	}
	if(token==NULL){
		printf(" starting_mf_log token is NULL\n"); return NULL;
	}
	char str[20];
	sprintf(str, "/new_log?code=%i",code);
	URL=concat_and_free(&URL, str);
	//we do not define the IP, then the server will use the one from the request
	URL=concat_and_free(&URL, "&message=\"");
	if(msg!=NULL){
		URL=concat_and_free(&URL, msg);
	}else{
		printf(" starting_mf_log msg is NULL\n"); return NULL;
	}
	URL=concat_and_free(&URL, "\"");

	URL=concat_and_free(&URL, "&user=\"");
	if(msg!=NULL){
		URL=concat_and_free(&URL, user);
	}else{
		printf(" starting_mf_log user is NULL\n"); return NULL;
	}
	URL=concat_and_free(&URL, "\"");
		printf("URL is %s\n",URL);

	char operation[]="POST";
	int result=query_message_json(URL, msg, NULL, &response, operation, token); //*****
	if(URL!=NULL) free(URL);
	URL=NULL;
	if(response.headercode!=NULL) free(response.headercode);
	response.headercode=NULL;
	if(result!=SUCCESS || response.data==NULL){

		printf("ERROR: Cannot register starting_mf_log ...\n");fflush(stdout);
		return NULL;
	}else if(response.data[0] == '\0') {
		printf("ERROR: Cannot register starting_mf_log\n");fflush(stdout);
		return NULL;
	}
	char* p= response.data;
	return p;
}


/** server consists on an address or ip with a port number like http://129.168.0.1:8600/ */
char *mf_start(const char *server, const char *exec_server, const char *exec_id, const char* resource_manager, const char *platform_id, metrics *m,struct app_report_t *my_app_report, const char *token) {
	long long int start_app_time = mycurrenttime();
	starting_mf_log(server, 200, "Call%20to%20mf_start", "non%20specified%20user", "token");
	my_app_report->start_app= start_app_time;
	char* resp= starting_exec(exec_server, exec_id, token);
	free(resp);
	/* get pid and setup the DataPath according to pid */
	pid = api_prepare(DataPath);
	/* get parameters from server with given platform_id */
	if(get_config_parameters(resource_manager, platform_id, token) != SUCCESS) {
		printf("ERROR : get_config_parameters failed.\n");
		return NULL;
	}
	//printf(" get_config_parameters succeed.\n");
// 	printf("num_threads %i\n",m->num_metrics);
// 	fflush(stdout);
	num_threads = m->num_metrics;
	if(num_threads+1 >=MAX_NUM_METRICS){
		printf(" error num_threads >=MAX_NUM_METRICS !!\n");
		return NULL;
	}
	unsigned int  t;
	int iret[num_threads];
	each_m=(struct each_metric_t **) malloc(num_threads*sizeof(struct each_metric_t *));
	if(each_m==NULL) {
		printf("Failed to allocate memory.\n");
		exit(1);
	}
	for(t=0;t<num_threads;t++)
		each_m[t]=(struct each_metric_t *) malloc(1*sizeof(struct each_metric_t));
	running = 1;
	keep_local_data_flag = m->local_data_storage;

	// !! mf_start_tid(pid, 420000);
	mmy_task_data_a=(struct task_data_t *) malloc(sizeof(struct task_data_t));
	init_stats(mmy_task_data_a);//this reserves the memory struct
	mmy_task_data_a->pid=pid;
	mmy_task_data_a->microsleep=420000;
	mmy_task_data_a->totaltid=0;
	mmy_task_data_a->total_user_def=0;
	for(t=0;t <mmy_task_data_a->maxprocesses; t++){//reserved positions
		mmy_task_data_a->task_def[t]->pstid=0;
	}

	for (t = 0; t < num_threads; t++) {
		/*prepare the argument for the thread*/
		each_m[t]->my_app_report=my_app_report;//all share the memory, but each one can write only their own fields !!!
		each_m[t]->start_app_time=start_app_time;
		each_m[t]->sampling_interval = m->sampling_interval[t];
		strcpy(each_m[t]->metric_name, m->metrics_names[t]);
		each_m[t]->my_task_data_a=mmy_task_data_a;//(struct task_data_t *) malloc(sizeof(struct task_data_t));
		each_m[t]->my_task_data_a->totaltid=0;

		/*create the thread and pass associated arguments */
		iret[t] = pthread_create(&threads[t], NULL, MonitorStart, (each_m[t]));// plus struct app_report_t *my_app_report, for add the power consumption !!
		if (iret[t]) {
			printf("ERROR: pthread_create failed for %s\n", strerror(iret[t]));
			if(each_m[t]!=NULL)
				free(each_m[t]);
			each_m[t]=NULL;
			return NULL;
		}
	}

	mmy_task_data_a->my_app_report=my_app_report;
// 	for(t=0;t <mmy_task_data_a->maxprocesses; t++){
// 		if(t<my_app_report->num_of_threads){
// 			printf(" \"ZZZZXXXXXZZZZ\":\"%i %s\"", t, my_app_report->my_thread_report[t]->taskid);
// 		}
// 	}

	for(t=0;t <mmy_task_data_a->maxprocesses; t++){
		if(t<my_app_report->num_of_threads){
			strcpy( mmy_task_data_a->subtask[t]->taskid, my_app_report->my_thread_report[t]->taskid);
		}else{
			strcpy( mmy_task_data_a->subtask[t]->taskid,"0");
		}
	}
	int tidret = pthread_create(&threads[num_threads], NULL, Monitor_tid_Start, mmy_task_data_a);// Monitor_tid_Start will increase the totaltid
	if (tidret) {
		printf("ERROR: pthread_create failed for %s\n", strerror(tidret));
		return NULL;
	}
	return DataPath;
}


/**
* Stop threads.
* Close all the files for data storage.
*/
void mf_end(void){
// 	int t;
	running = 0;
// 	for (t = 0; t < num_threads+1; t++) //we add one more for the Monitor_tid_Start
// 		pthread_join(threads[t], NULL);
// 	int totalfree=0;
// 	if(each_m!=NULL){
// 		for (t = 0; t < num_threads; t++){
// 			if(each_m[t]!=NULL)
// 				free(each_m[t]);
// 			each_m[t]=NULL;
// 			totalfree++;
// 		}
// 		if(totalfree==num_threads && each_m!=NULL)
// 			free(each_m);
// 		each_m=NULL;
// 	}
	close_curl();
	printf("finished mf_end\n");
}

/**
* Query for a workflow, return 400 if the workflow is not registered yet.
* or 200 in other case.
* @return NULL if error
*/
char* mf_query_workflow(const char *server, const char *application_id){
	/* create an workflow */
	char *URL = NULL;
	struct url_data response;
	response.size=0;
	response.data=NULL;
	response.headercode=NULL;
	if(server!=NULL){
		URL=concat_and_free(&URL, server);
	}else{
		printf(" mf_query_workflow server is NULL\n");return NULL;
	}
	URL=concat_and_free(&URL, "/v1/phantom_mf/workflows/");
	URL=concat_and_free(&URL, application_id);
	if(application_id!=NULL){
		URL=concat_and_free(&URL, application_id);
	}else{
		printf(" mf_query_workflow application_id is NULL\n");
	}
//	printf("******* register workflow ******\n");
	char operation[]="GET";
//	new_query_json(URL, &response, operation,NULL); //********2 times ??? ****************************************
	if(new_query_json(URL, &response, operation, NULL) > 0) {
		printf("ERROR: query GET \"%s\" failed.\n", URL);
		if(URL!=NULL) free(URL);
		URL=NULL;
		if(response.data!=NULL) { free(response.data); response.data=NULL; }
		if(response.headercode!=NULL) { free(response.headercode); response.headercode = NULL; }
		return NULL;
	}
	if(URL!=NULL) free(URL);
	URL=NULL;
	if(response.data[0] == '\0') {
		printf("ERROR: Cannot register workflow for application %s\n", application_id);
		if(response.data!=NULL) free(response.data);
		response.data=NULL;
		if(response.headercode!=NULL) free(response.headercode);
		response.headercode=NULL;
		return NULL;
	}
	if(response.data!=NULL) free(response.data);
	response.data=NULL;
	return response.headercode;
}


/**
* Register a new workflow.
* @return the path to query the workflow.
*/
char* mf_new_workflow(const char *server, const char *application_id, const char *author_id,
	const char *optimization, const char *tasks_desc, const char *token) {
	/* create an workflow */
	char *URL = NULL;
	struct url_data response;
	response.size=0;
	response.data=NULL;
	response.headercode=NULL;
	char *msg = NULL;
	msg=concat_and_free(&msg, "{\"application\":\"");
	msg=concat_and_free(&msg, application_id);
	msg=concat_and_free(&msg, "\", \"author\": \"");
	msg=concat_and_free(&msg, author_id);
	msg=concat_and_free(&msg, "\", \"optimization\": \"");
	msg=concat_and_free(&msg, optimization);
	msg=concat_and_free(&msg, "\", \"tasks\": ");
	msg=concat_and_free(&msg, tasks_desc);
	msg=concat_and_free(&msg, "}");
	URL=concat_and_free(&URL, server);
	URL=concat_and_free(&URL, "/v1/phantom_mf/workflows/");
	URL=concat_and_free(&URL, application_id);
//	printf("******* register workflow ******\n");
	char operation[]="PUT";
	query_message_json(URL, msg, NULL, &response, operation, token); //*****
	if(msg!=NULL) free(msg);
	msg=NULL;
	if(URL!=NULL) free(URL);
	URL=NULL;
	if(response.headercode!=NULL) free(response.headercode);
	response.headercode=NULL;
	if(response.data[0] == '\0') {
		printf("ERROR: Cannot register workflow for application %s\n", application_id);
		return NULL;
	}
	char* p= response.data;
	return p;
}


/**
* Generate the execution_id.
* Send the monitoring data in all the files to mf_server.
*@return the execution_id
*/
char* mf_send(const char *server,const char *application_id,const char *component_id,const char *platform_id,const char *token) {
	starting_mf_log(server, 200, "Call%20to%20mf_send", "non%20specified%20user", "token");
	/* create an experiment with regards of given application_id, component_id and so on */
	char *URL = NULL;
	struct url_data response;
	response.size=0;
	response.data=NULL;
	response.headercode=NULL;
//	char *experiment_id = NULL;
	char *msg = NULL;
	msg=concat_and_free(&msg, "{\"application\":\"");
	msg=concat_and_free(&msg, application_id);
	if(component_id==NULL){
		printf(" mf_send component_id is NULL\n");
	}else{
		msg=concat_and_free(&msg, "\", \"task\": \"");
		msg=concat_and_free(&msg, component_id);
	}
	if(platform_id==NULL){
		printf(" mf_send platform_id is NULL\n");
	}else{
		msg=concat_and_free(&msg, "\", \"host\": \"");
		msg=concat_and_free(&msg, platform_id);
	}
	msg=concat_and_free(&msg, "\"}");
	if(server!=NULL){
		URL=concat_and_free(&URL, server);
	}else{
		printf(" mf_send server is NULL\n");return NULL;
	}
	if(token==NULL){
		printf(" mf_send token is NULL\n");return NULL;
	}
	URL=concat_and_free(&URL, "/v1/phantom_mf/experiments/");
	URL=concat_and_free(&URL, application_id);
//	printf("******* new_create_new_experiment ******\n");
	char operation[]="POST";
	if(query_message_json(URL, msg, NULL, &response, operation, token)==FAILED){
		printf("ERROR: Cannot create new experiment for application %s, failed response\n", application_id);
		if(msg!=NULL) {free(msg); msg=NULL;}
		if(URL!=NULL) {free(URL); URL=NULL;}
		if(response.data!=NULL) {free(response.data); response.data=NULL;}
		if(response.headercode!=NULL) {free(response.headercode); response.headercode=NULL;}
		return NULL;
	}
	if(msg!=NULL) {free(msg); msg=NULL;}
	if(URL!=NULL) {free(URL); URL=NULL;}
	if(response.headercode!=NULL) {free(response.headercode); response.headercode=NULL;}
	if(response.data==NULL){
		printf("ERROR: on response.data when creating new experiment for application %s\n", application_id);
		return NULL;
	}
	if(response.data[0] == '\0') {//response->data containes the experiment_id
		printf("ERROR: Cannot create new experiment for application %s\n", application_id);
		return NULL;
	}
	/*malloc variables for send metrics */
	char *metric_URL = (char *) malloc(200);
	char *static_string = (char *) malloc(200);
	char *filename = (char *) malloc(200);
	if(filename==NULL) {
		printf("Failed to allocate memory.\n");
		exit(1);
	}
	metric_URL[0]='\0';
	metric_URL=concat_and_free(&metric_URL, server);
	metric_URL=concat_and_free(&metric_URL, "/v1/phantom_mf/metrics");
	//processing all the files in the directory, and forward to the Monitoring-Server
	struct dirent *drp;
	DIR *dir = opendir(DataPath);
	if(dir == NULL) {
		printf("Error: Cannot open directory %s\n", DataPath);
		return NULL;
	}
	while ((drp = readdir(dir)) != NULL){
		static_string[0]='\0';//in the next loop the string will be empty
		//memset(static_string, '\0', malloc_usable_size(static_string));//this is too much, not need that effort
		filename[0]='\0';//in the next loop the string will be empty
		//memset(filename, '\0', malloc_usable_size(filename)); //this is too much, not need that effort
		if(drp->d_name[0]!='.'){//not wish to process (. .. or hidden files)
			filename=concat_and_free(&filename, DataPath);
			filename=concat_and_free(&filename, "/");
			if(drp->d_name!=NULL){
				filename=concat_and_free(&filename, drp->d_name);
			}else{
				printf(" mf_send drp->d_name is NULL\n");
			}
			static_string=concat_and_free(&static_string, "\"WorkflowID\":\"");//****
			static_string=concat_and_free(&static_string, application_id);
			static_string=concat_and_free(&static_string, "\", \"TaskID\": \"");
			static_string=concat_and_free(&static_string, component_id);
			static_string=concat_and_free(&static_string, "\", \"ExperimentID\": \"");//****
			static_string=concat_and_free(&static_string, response.data);//experiment_id
			static_string=concat_and_free(&static_string, "\", \"type\": \"");
			static_string=concat_and_free(&static_string, drp->d_name);
			static_string=concat_and_free(&static_string, "\", \"host\": \"");
			static_string=concat_and_free(&static_string, platform_id);
			static_string=concat_and_free(&static_string, "\"");
			publish_file(metric_URL, static_string, filename, token);
			/*remove the file if user unset keep_local_data_flag */
			if(keep_local_data_flag == 0)
				unlink(filename);
		}
	}
	closedir(dir);

	if(metric_URL!= NULL)
		free(metric_URL);
	metric_URL=NULL;
	if(static_string!= NULL)
		free(static_string);
	static_string=NULL;
	if(filename!= NULL)
		free(filename);
	filename=NULL;
	if(logFile != NULL)
		fclose(logFile);
	logFile=NULL;


//		for (t = 0; t < num_threads; t++) {
//			for(i=0;i<each_m[t]->my_task_data_a.maxprocesses;i++)
//				free(each_m[t]->my_task_data_a.subtask[i]);
//			free(each_m[t]->my_task_data_a.cores);
//			free(each_m[t]->my_task_data_a.subtask);
//		}

	/*remove the data directory if user unset keep_local_data_flag */
	if(keep_local_data_flag == 0)
		rmdir(DataPath);
	return response.data; // it contains the experiment_id;
}

/**
* Get the pid, and setup the DataPath for data storage
*/
static int api_prepare(char *Data_path){
	/*reset Data_path*/
	size_t size = strlen(Data_path);
	memset(Data_path, '\0', size);
	/*get the pid*/
	int pid = getpid();
	/*get the pwd*/
	char buf_1[256] = {'\0'};
	char buf_2[256] = {'\0'};
	int ret = readlink("/proc/self/exe", buf_1, 200);
	if(ret == -1) {
		printf("readlink /proc/self/exe failed.\n");
		exit(0);
	}
	memcpy(buf_2, buf_1, strlen(buf_1) * sizeof(char));
	/* extract path folder of executable from it's path */
	char *lastslash = strrchr(buf_2, '/');
	int ptr = lastslash - buf_2;
	memcpy(Data_path, buf_2, ptr);
	/*create logfile*/
	char logFileName[256] = {'\0'};
	sprintf(logFileName, "%s/log_%i.txt", Data_path, pid);
	logFile = fopen(logFileName, "w");
	if (logFile == NULL)
		printf("Could not create log file %s", logFileName);
	/*create the folder with regards of the pid*/
	sprintf(Data_path + strlen(Data_path), "/%d", pid);
	struct stat st = { 0 };
	if (stat(Data_path, &st) == -1)
		mkdir(Data_path, 0700);
	return pid;
}





static void *MonitorStart(void *arg) {
	each_metric *metric = (each_metric*) arg;
//	printf("Starting monitoring modules\n");
	if(strcmp(metric->metric_name, METRIC_NAME_1) == 0) {
//		printf("Starting monitoring modul %s\n",METRIC_NAME_1);
		stat_resources=linux_resources(pid, DataPath, metric->sampling_interval);
	} else if(strcmp(metric->metric_name, METRIC_NAME_2) == 0) {
//		printf("Starting monitoring modul %s\n",METRIC_NAME_2);
		xlnx_monitor(DataPath, metric->sampling_interval);
	} else if(strcmp(metric->metric_name, METRIC_NAME_3) == 0) {
//  		printf("Starting monitoring modul %s\n",METRIC_NAME_3);
		power_monitor(pid, DataPath, metric->sampling_interval, metric->start_app_time, metric->my_app_report, metric->my_task_data_a); //struct app_report_t *my_app_report
#ifdef NVML
#if NVML == yes
	} else if(strcmp(metric->metric_name, METRIC_NAME_4) == 0) {
//		printf("Starting monitoring modul %s\n",METRIC_NAME_4);
		nvml_monitor(pid, DataPath, metric->sampling_interval);
#endif
#endif
	} else {
		printf("ERROR: it is not possible to monitor %s\n", metric->metric_name);
		printf(" available metric plugins:\n");
		printf(" \"%s\", \"%s\", \"%s\"", METRIC_NAME_1, METRIC_NAME_2, METRIC_NAME_3);
#ifdef NVML
#if NVML == yes
		printf(", \"%s\"", METRIC_NAME_4);
#endif
#endif
		printf("\n");
	}
	return NULL;
}

void debugging_mf_configs(struct json_mf_config **mf_config, const unsigned int total_loaded_mf_configs){
//	print_counters_from_json(html);
//	print_counters_from_parsed_json(mf_config, total_loaded_mf_configs);
//	print_json(html);
	print_parsed_json(mf_config,total_loaded_mf_configs);
}


/**
* @return SUCCESS(0) if succeed, otherwise returns FAILED
*/
int get_config_parameters(const char *resource_manager, const char *platform_id, const char *token){
		return SUCCESS;
	/* send the query and retrieve the response string */
//	char resoucemanager_path[]="query_device_mf_config?pretty=true\\&device=\"";//pretty is not needed
	if(strlen(platform_id)==0)
		return 1;//we can not proceed without a platform_id

	unsigned int total_loaded_mf_configs =0;
	struct json_mf_config **mf_config=NULL;
//	const char *resource_manager="141.58.0.8:2780";//examples: 141.58.0.8:2780 or localhost:8600

	char *html=query_mf_config(resource_manager, platform_id, token);
	parse_mf_config_json(html, &mf_config, &total_loaded_mf_configs);
	if(html!=NULL) free(html);
	//****************** END OF LOADING THE MF_CONFIG FROM RESOURCE MANAGER, ALREADY PARSED*****/
	int pos= query_for_platform_parsed_json(platform_id, mf_config,total_loaded_mf_configs);
	printf(" found node: %s in position %i\n", platform_id, pos);

	char search_plugin[]="mf_plugin_board_power";
	char *response_query = query_for_plugin_parsed_json(0, search_plugin, mf_config,total_loaded_mf_configs);
	printf(" found plugin: %s%s%s as %s%s%s\n", yellow,search_plugin, NO_COLOUR, LIGHT_BLUE,
		 (response_query==NULL)?"NULL":response_query, NO_COLOUR);
	if(response_query!=NULL) free(response_query);
	//**************************** END OF PARSING THE MF_CONFIG ************/
	debugging_mf_configs(mf_config, total_loaded_mf_configs);
	//free mf_config memory
	unsigned int num,counter, total_fields,total_objects;
	for(num =0; num <total_loaded_mf_configs;num++){
		for(total_fields =0; total_fields<mf_config[num]->count_f; total_fields++){
			total_objects= mf_config[num]->field[total_fields]->count_o;
			for(counter=0;counter< total_objects ;counter++){
				free(mf_config[num]->field[total_fields]->object[counter]->label_o);
				free(mf_config[num]->field[total_fields]->object[counter]->value_o);
				free(mf_config[num]->field[total_fields]->object[counter]);
			}
			free(mf_config[num]->field[total_fields]->object);
			free(mf_config[num]->field[total_fields]->label_f);
			free(mf_config[num]->field[total_fields]);
		}
		free(mf_config[num]->field);
		free(mf_config[num]);
	}
	free(mf_config);

//	unsigned int need_string_size=strlen(server)+ strlen(resoucemanager_path)+ strlen(platform_id)+ strlen("\"")+1;
//	if (server[strlen(server)] != '/' )
//		need_string_size++;// we need to add an slash "/"
//	char *URL =(char *) malloc(need_string_size);
//	if(URL==NULL) {
//		printf("Failed to allocate memory.\n");
//		exit(1);
//	}
//	strcpy(URL,server);
//	if (server[strlen(server)] != '/' )
//		strcat(URL,"/"); // we need to add an slash "/"
//	strcat(URL,resoucemanager_path);
//	strcat(URL,platform_id);
//	strcat(URL,"\"");
//	struct url_data response; //it is reserved by new_query_json, new_query_json is defined in publisher.c
//	response.data=NULL;
//	response.headercode = NULL;
//	char operation[]="GET";
//	int mycode =new_query_json(URL, &response, operation,token);
//	if(mycode > 0) {
//		printf("ERROR: query with %s failed code %i.\n", URL, mycode);
//		if(URL!=NULL) free(URL);
//		URL=NULL;
//		if(response.data!=NULL) { free(response.data); response.data=NULL; }
//		if(response.headercode!=NULL) { free(response.headercode); response.headercode = NULL; }
//		return FAILED;
//	}

//	if(strstr(response.data, "parameters") == NULL) {
//		printf("ERROR: response does not include parameters.\n");
//		printf("response.data: %s.\n",response.data);
//		printf("GET URL: %s.\n",URL);
//		if(URL!=NULL) { free(URL); URL=NULL; }
//		if(response.data!=NULL) { free(response.data); response.data=NULL; }
//		if(response.headercode!=NULL) { free(response.headercode); response.headercode = NULL; }
//		return FAILED;
//	}
//	if(URL!=NULL) { free(URL); URL=NULL; }
	/* parse the send back string to get required parameters */

/*	char *ptr_begin, *ptr_end;
	char value[16] = {'\0'};
	int i, value_length;
	for (i = 0; i <= 8; i++) {
		ptr_begin = strstr(response.data, parameters_name[i]);
		if(ptr_begin != NULL) {
			ptr_end = strstr(ptr_begin, ",");
			if(ptr_end == NULL) {
				ptr_end = strstr(ptr_begin, "}");
				if(ptr_end == NULL){
					if(response.data!=NULL) { free(response.data); response.data=NULL; }
					if(response.headercode!=NULL) { free(response.headercode); response.headercode = NULL; }
					return FAILED;
				}
			}
			value_length = ptr_end - ptr_begin - 4 - strlen(parameters_name[i]);
			ptr_begin += 3 + strlen(parameters_name[i]);
			memset(value, '\0', 16);
			strncpy(value, ptr_begin, value_length);
			parameters_value[i] = atof(value);
		}
	}
	printf("Parameters of platform \"%s\" are:\n",platform_id);
	for (i = 0; i <= 8; i++)
		printf("    %s:%f\n", parameters_name[i], parameters_value[i]);
	if(response.data!=NULL) { free(response.data); response.data=NULL; }
	if(response.headercode!=NULL) { free(response.headercode); response.headercode = NULL; }*/
	return SUCCESS;
}


/**
* NEW FUNCTIONS DEVELOPED DURING THE INTEGRATION
*/

metric_query *new_metric(const char* label){
	metric_query *user_query;
	user_query = (metric_query *) malloc(sizeof(struct metric_query_t));
	user_query->query=NULL;
	user_query->multiple_fields=0;
	user_query->query=concat_and_free(&user_query->query, "\"");
	user_query->query=concat_and_free(&user_query->query, label);
	user_query->query=concat_and_free(&user_query->query, "\":");
	return user_query;
}


metric_query *add_int_field(metric_query* user_query, const char* label, const unsigned int total, int array_int[]) {
	unsigned int i;
	char new_string[20];
	if(label[0]=='\0'){
		if(total>1)
			user_query->query=concat_and_free(&user_query->query, "[");
		for(i=0;i<total;i++){
			if(i>0) user_query->query=concat_and_free(&user_query->query, ",");
			user_query->query=concat_and_free(&user_query->query, itoa((int) array_int[i], new_string));
		}
		if(total>1)
			user_query->query=concat_and_free(&user_query->query, "]");
	}else{
		if(user_query->multiple_fields==0){
			user_query->query=concat_and_free(&user_query->query, "{\n\t");
			user_query->multiple_fields=1;
		}else{
			user_query->query=concat_and_free(&user_query->query, ",\n\t");
		}
		user_query->query=concat_and_free(&user_query->query, "\"");
		user_query->query=concat_and_free(&user_query->query, label);
		user_query->query=concat_and_free(&user_query->query, "\":");
		user_query =add_int_field(user_query, "", total, array_int);
	}
	return user_query;
}

metric_query *add_string_field(metric_query* user_query, const char* label, const unsigned int total, char **array_str ) {
	unsigned int i;
	if(label[0]=='\0'){
		if(total>1)
			user_query->query=concat_and_free(&user_query->query, "[");
		for(i=0;i<total;i++){
			if(i>0) user_query->query=concat_and_free(&user_query->query, ",");
			user_query->query=concat_and_free(&user_query->query, "\"");
			user_query->query=concat_and_free(&user_query->query, array_str[i]);
			user_query->query=concat_and_free(&user_query->query, "\"");
		}
		if(total>1)
			user_query->query=concat_and_free(&user_query->query, "]");
	}else{
		if(user_query->multiple_fields==0){
			user_query->query=concat_and_free(&user_query->query, "{\n\t");
			user_query->multiple_fields=1;
		}else{
			user_query->query=concat_and_free(&user_query->query, ",\n\t");
		}
		user_query->query=concat_and_free(&user_query->query, "\"");
		user_query->query=concat_and_free(&user_query->query, label);
		user_query->query=concat_and_free(&user_query->query, "\":");
		user_query =add_string_field(user_query, "", total, array_str);
	}
	return user_query;
}


void submit_metric(metric_query *user_query){
//	long long int start_time=mycurrenttime(); <-- its purpose is to add automatically the local timestamp on every collected metric
	if(user_query->multiple_fields==1)
		user_query->query=concat_and_free(&user_query->query, "\n}");
	printf("%s\n\n",user_query->query);
	if(user_query->query!=NULL) free(user_query->query);
	if(user_query!=NULL) free(user_query);
	user_query=NULL;
}


void submit_metric_json(char *user_string){
	printf("%s\n\n",user_string);
}


int mf_user_metric_loc_type(char *user_metrics, const char *datatype){
	if(DataPath[0] == '\0')
		pid = api_prepare(DataPath);
	/*create and open the file*/
	char FileName[256] = {'\0'};
	sprintf(FileName, "%s/%s", DataPath, datatype);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		return 0;
	}
	struct timespec timestamp;
	/*get current timestamp */
	clock_gettime(CLOCK_REALTIME, &timestamp);
	/*convert to milliseconds */
	double timestamp_ms = timestamp.tv_sec * 1000.0 + (double)(timestamp.tv_nsec / 1.0e6);
	fprintf(fp, "\"local_timestamp\":\"%.1f\"", timestamp_ms);
	fprintf(fp, "%s\n",user_metrics);
	/*close the file*/
	fclose(fp);
	return pid;
}

int mf_user_metric_loc(char *user_metrics ){
	int result = mf_user_metric_loc_type(user_metrics ,"user_defined" );
	return result;
}


void user_metrics_buffer(struct Thread_report_t single_thread_report ){
	unsigned int j;
/* MONITORING I'd like to store here the duration of each loop --> duration */
	char run_id[]="runid";
	char *user_metrics= (char *) malloc(510);
	if(user_metrics==NULL)
		fprintf(stderr, "Failed to allocate memory.\n");
	for(j=0;j<single_thread_report.total_metrics;j++){
		user_metrics[0]='\0';
		// common id of the different components of one execution of the task/workflow/application
		user_metrics=myconcat(&user_metrics,",\"", run_id, "\":\"", single_thread_report.currentid,"\"");
		user_metrics=myconcat(&user_metrics,",\"", "component_name", "\":\"", single_thread_report.taskid,"\"");
		user_metrics=myconcat(&user_metrics,",\"", "metric_time", "\":\"", single_thread_report.metric_time[j],"\"");
		user_metrics=myconcat(&user_metrics,",\"", single_thread_report.user_label[j], "\": ", single_thread_report.user_value[j],"");

	/* MONITORING END */
		mf_user_metric_loc(user_metrics);
	}
	if(user_metrics!=NULL) free(user_metrics);
	user_metrics=NULL;
///* MONITORING I'd like to store here the total nr. of completed loops --> nrLoops */
//	int nrLoops=5;
//	sprintf(metric_value, "%d", nrLoops);
//	char nrLoops_str[]="nrLoops";
//	mf_user_metric_loc(nrLoops_str, metric_value);
/* MONITORING SEND */
}


void register_end_component(char *currentid, struct Thread_report_t single_thread_report){
	long long int total_execution_time_us = (single_thread_report.end_time - single_thread_report.start_time)/1000;
// 	long long int total_execution_time_us = (time_of_last_measured - single_thread_report.start_time)/1000; //will be convenient to compare with time_of_last_measured??
	//from mmy_task_data_a->subtask[i]->time_of_last_measured
//	printf(" TOTAL EXECUTION TIME:%9Li s ", (total_execution_time)/1000000000LL);
//	long long int temp_time = (total_execution_time)%1000000000LL;
//	printf(" + %3Li ms ", temp_time/1000000);
//		temp_time = (total_execution_time)%1000000LL;
//	printf(" + %3Li us ", temp_time/1000);
//		temp_time = (total_execution_time)%1000LL;
//	printf(" + %3Li ns \n", temp_time);
/* MONITORING I'd like to store here the duration of each loop --> duration */
	char metric_value[100];
	//	sprintf(metric_value, "%Li", total_execution_time);
	llint_to_string_alloc(total_execution_time_us, metric_value);
	char comp_start[]="component_start";
	char comp_end[]="component_end";
	char duration_str[]="component_duration";
	char taskid_str[]="component_name";
//	char user_defined_metric[]="user_defined_metric";

	char run_id[]="runid";

	char *user_metrics= (char *) malloc(510);
	if(user_metrics==NULL)
		fprintf(stderr, "Failed to allocate memory.\n");
	user_metrics[0]='\0';
		// the total execution time of the component
	user_metrics=myconcat(&user_metrics,",\"", duration_str, "\":\"", metric_value,"\"");
		// common id of the different components of one execution of the task/workflow/application
	user_metrics=myconcat(&user_metrics,",\"", taskid_str, "\":\"", single_thread_report.taskid ,"\"");//patch for D2.2
	user_metrics=myconcat(&user_metrics,",\"", run_id, "\":\"", currentid,"\"");
	char strstart_time[100];
	llint_to_string_alloc(single_thread_report.start_time,strstart_time);
	user_metrics=myconcat(&user_metrics,",\"", comp_start, "\":\"", strstart_time, "\"");
		// end time of the component
	char strend_time[100];
	llint_to_string_alloc(single_thread_report.end_time, strend_time); //will be convenient to compare with time_of_last_measured??
	user_metrics=myconcat(&user_metrics,",\"", comp_end, "\":\"", strend_time, "\"");

/* MONITORING END */
	mf_user_metric_loc_type( user_metrics, "duration_components");
	if(user_metrics!=NULL) free(user_metrics);
	user_metrics=NULL;
///* MONITORING I'd like to store here the total nr. of completed loops --> nrLoops */
//	int nrLoops=5;
//	sprintf(metric_value, "%d", nrLoops);
//	char nrLoops_str[]="nrLoops";
//	mf_user_metric_loc(nrLoops_str, metric_value);
/* MONITORING SEND */
}

//it calls the mf_end and mf_send and frees memory, and closes logfile
void monitoring_end(const char *mf_server, const char *exec_server, const char *appid, const char *exec_id, const char *execfile,
		const char *regplatformid, const char *token, struct app_report_t *my_app_report){
//	for (int t = 0; t < num_threads; t++) {
//		if(each_m!=NULL)
//		if(each_m[t]!=NULL){
//			for (int kk =0; kk < each_m[t]->my_task_data_a->totaltid;kk++){
//			printf("******* ZZZZ %i \"cpu_energy [%i]\":\"%.2f\"\n", t,kk, each_m[t]->my_task_data_a->subtask[kk]->total_cpu_energy);
//			printf("******* ZZZZ %i \"cpu_energy [%i]\":\"%.2f\"\n", t,kk, mmy_task_data_a->subtask[kk]->total_cpu_energy);
//			}
//	}}


	starting_mf_log(mf_server, 200, "Call%20to%20monitoring_end", "non%20specified%20user", "token");

	mf_end();
	/* MONITORING SEND */
	char *experiment_id = mf_send(mf_server, appid, execfile, regplatformid, token);
//	printf(" experiment_id is %s\n",experiment_id);
	if(experiment_id!=NULL) free(experiment_id);//dynamically allocated by mf_send
	experiment_id=NULL;
	if(my_app_report==NULL) {
		printf("my_app_report NULL!!\n");
		for(unsigned int i=0;i<mmy_task_data_a->maxprocesses;i++){
			free(mmy_task_data_a->subtask[i]);
			free(mmy_task_data_a->task_def[i]);
		}
		free(mmy_task_data_a->cores);
		free(mmy_task_data_a->subtask);
		free(mmy_task_data_a->task_def);
		if(mmy_task_data_a!=NULL)
			free(mmy_task_data_a);
		mmy_task_data_a=NULL;
		return;
	}

	//SAVE the FILE for forwarding it to the Execution Manager
	char FileName[256] = {'\0'};
	sprintf(FileName, "exec_stats_%i.json", pid);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
// 			if(my_app_report!=NULL)
// 				free_app_report(my_app_report);
// 			my_app_report=NULL;
// 			for(int i=0;i<mmy_task_data_a->maxprocesses;i++){
// 				free(mmy_task_data_a->subtask[i]);
// 				free(mmy_task_data_a->task_def[i]);
// 			}
// 			free(mmy_task_data_a->cores);
// 			free(mmy_task_data_a->subtask);
// 			free(mmy_task_data_a->task_def);
// 			if(mmy_task_data_a!=NULL)
// 				free(mmy_task_data_a);
// 			mmy_task_data_a=NULL;
		return;
	}

	char *json_msg=mf_exec_stats(*my_app_report, appid, exec_id, regplatformid, mmy_task_data_a);
	if(json_msg!=NULL)
	fprintf(fp,"%s",json_msg);
	/*close the file*/
	fclose(fp);

	char* resp= update_exec(exec_server, FileName, token);// this sends the file to the server.
//	if (resp!= NULL)//if NULL the data was NOT uploaded to the server, may because the server is not reachable
//		unlink(FileName); //delete the temporal file with statistics
	free(resp);
	free(json_msg);
	printf("\n");

	printf("*** total threads %i \n",my_app_report->num_of_threads);
	for(unsigned int i=0;i<my_app_report->num_of_threads;i++){

// 		long long int start_time_ns = my_app_report->my_thread_report[i]->start_time;
// 		if(start_time_ns!=0){
			printf(" Execution label of the workflow: \"%s\"\n", my_app_report->currentid);
			printf("   THREAD num %i, name : %s\n",i, my_app_report->my_thread_report[i]->taskid);
			printf("     total metrics %i:\n",my_app_report->my_thread_report[i]->total_metrics);
			for(unsigned int j=0;j<my_app_report->my_thread_report[i]->total_metrics;j++)
				printf("   %s : %s\n", my_app_report->my_thread_report[i]->user_label[0], my_app_report->my_thread_report[i]->user_value[0]);
			printf("   duration of the component : %lli us\n", (mmy_task_data_a->subtask[i]->time_of_last_measured- mmy_task_data_a->subtask[i]->start_comp)/1000);
			printf("   duration of the component_b : %lli us\n", (my_app_report->my_thread_report[i]->end_time- my_app_report->my_thread_report[i]->start_time)/1000);
	//		printf(" MM [ %lli  %lli %lli %lli]\n",
	//		mmy_task_data_a->subtask[i]->start_comp,
	//		my_app_report->my_thread_report[i]->start_time,
	//		my_app_report->my_thread_report[i]->end_time,
	//		mmy_task_data_a->subtask[i]->time_of_last_measured);
// 		}
	}

// NEED before free the memory to confirm first that all threads finished, or we will get segmentation fault on their side !!!
// 	if(my_app_report!=NULL)
// 		free_app_report(my_app_report);
// 	my_app_report=NULL;
//
// 	for(int i=0;i<mmy_task_data_a->maxprocesses;i++){
// 		free(mmy_task_data_a->subtask[i]);
// 		free(mmy_task_data_a->task_def[i]);
// 	}
// 	free(mmy_task_data_a->cores);
// 	free(mmy_task_data_a->subtask);
// 	free(mmy_task_data_a->task_def);
// 	if(mmy_task_data_a!=NULL)
// 		free(mmy_task_data_a);
// 	mmy_task_data_a=NULL;
}


/**
* this function query for a workflow was registered or not
* if the status code is 400 means that it was not registered before*/
char* query_workflow(const char *server, const char *appid){
	char* response=response=mf_query_workflow( server, appid );
	return response;
}

/** @return 0 if success*/
int register_workflow(const char *server, const  char *appid,  const char *token){
	char author[]="new_user";
	char optimization[]="Time";
	char tasks_desc[]="[{\"device\":\"demo_desktop\", \"exec\":\"hello_world\", \"cores_nr\": \"2\"}]";
//	printf("\nQUERY FOR WORKFLOW ...\n");
	char* response=query_workflow(server, appid);
	if (response==NULL) {
		printf("ERROR registering workflow server: %s\n",server);
		return 1;
	}
	int rc = strcmp(response, "400"); //if 400 then the workflow was not registered before
	if(rc == 0){ //0 stands for equal
		printf("free reposnes\n");
		if(response!=NULL) free(response);
//		printf("\nREGISTERING WORKFLOW ...\n");
		response= mf_new_workflow(server, appid, author, optimization, tasks_desc, token);
	}
	if(response!=NULL) free(response);
	return 0;
}
