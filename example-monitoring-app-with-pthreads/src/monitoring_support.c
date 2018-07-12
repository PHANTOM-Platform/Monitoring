#include <unistd.h>
#include <sys/stat.h>
// #include <cstring> //only in the c++ version
// #include <string> //only in the c++ version
#include <string.h>
#include <malloc.h>
#include "monitoring_support.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
// extern "C" {
	#include "mf_api.h"
// }

int pid;
char DataPath[256];

//Function for increase dynamically a string concatenating strings at the end
//It free the memory of the first pointer if not null
char* myconcat(char **s1, const char *a1, const char *a2, const char *a3, const char *a4, const char *a5)
{
	char *result = NULL;
	unsigned int new_lenght= strlen(a1)+strlen(a2)+strlen(a3)+strlen(a4)+strlen(a5)+1; //+1 for the null-terminator;
	if(*s1 != NULL){
		new_lenght+= strlen(*s1);//current lenght
		if(new_lenght> malloc_usable_size(*s1)){
			result = (char *) malloc(new_lenght);
			if(result==NULL) {
				fprintf(stderr, "Failed to allocate memory.\n");
				return 0;
			}
			strcpy(result, *s1);
			free(*s1);
		}else{
			result = *s1;
		}
	}else{
		result = (char *) malloc(new_lenght);
		if(result==NULL) {
			fprintf(stderr, "Failed to allocate memory.\n");
			return 0;
		}
		result[0]='\0';
	} 
	strcat(result, a1);
	strcat(result, a2);
	strcat(result, a3);
	strcat(result, a4);
	strcat(result, a5);
	return result;
}


//Function for increase dynamically a string concatenating strings at the end
//It free the memory of the first pointer if not null
char* concat_and_free(char **s1, const char *s2){
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

char* itoa(int i, char b[]){
	char const digit[] = "0123456789";
	char* p = b;
	if(i<0){
		*p++ = '-';
		i *= -1;
	}
	int shifter = i;
	do{ //Move to where representation ends
		++p;
		shifter = shifter/10;
	}while(shifter);
	*p = '\0';
	do{ //Move back, inserting digits as u go
		*--p = digit[i%10];
		i = i/10;
	}while(i);
	return b;
}



char* llitoa(const long long int i, char b[]){
	char const digit[] = "0123456789";
	char* p = b;
	long long int input = i;
	if(input<0){
		*p++ = '-';
		input *= -1;
	}
	int shifter = input;
	do{ //Move to where representation ends
		++p;
		shifter = shifter/10;
	}while(shifter);
	*p = '\0';
	do{ //Move back, inserting digits as u go
		*--p = digit[input%10];
		input = input/10;
	}while(input);
	return b;
}


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


metric_query *add_int_field(metric_query* user_query, const char* label, const unsigned int total, int array_int[] ) {
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
			user_query->query=concat_and_free(&user_query->query, array_str[i] );
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
// 	long long int start_time=mycurrenttime(); <-- its purpose is to add automatically the local timestamp on every collected metric
	if(user_query->multiple_fields==1)
		user_query->query=concat_and_free(&user_query->query, "\n}");
	printf("%s\n\n",user_query->query);
	free(user_query->query);
	free(user_query);
	user_query=NULL;
}


void submit_metric_json(char *user_string){
	printf("%s\n\n",user_string);
}

/**
*	Get the pid, and setup the DataPath for data storage
*/
static int api_prepare_loc(char *Data_path){
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

	/*create the folder for collect the date of the pid-process*/
	sprintf(Data_path + strlen(Data_path), "/%d", pid);
	struct stat st = { 0 };
	if (stat(Data_path, &st) == -1)
	mkdir(Data_path, 0700);
	return pid;
}

int mf_user_metric_loc(char *user_metrics){
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
	double timestamp_ms = timestamp.tv_sec * 1000.0 + (double)(timestamp.tv_nsec / 1.0e6); 
	fprintf(fp, "\"local_timestamp\":\"%.1f\"", timestamp_ms);
	fprintf(fp, "%s\n",user_metrics);
	/*close the file*/
	fclose(fp);
	return pid;
}

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
	long long int timeus = (long int) (ts_start.tv_sec*1000000000LL + ts_start.tv_nsec);
	return timeus;
}

char *mycurrenttime_str (void) {
	long long int mytime = mycurrenttime();
	char *new_string = (char *) malloc(60);
	new_string=llitoa(mytime,new_string);
	return new_string;
}

void start_monitoring( const char *server, const char *regplatformid){
	DataPath[0] = '\0';
/* MONITORING METRICS */
	metrics m_resources;
	m_resources.num_metrics = 3;
	m_resources.local_data_storage = 1; /*remove the file if user unset keep_local_data_flag */
	m_resources.sampling_interval[0] = 10; // 1000 stands for 1s
	strcpy(m_resources.metrics_names[0], "resources_usage");
	m_resources.sampling_interval[1] = 10; // 1s
	strcpy(m_resources.metrics_names[1], "disk_io");
	m_resources.sampling_interval[2] = 10; // 1s
	strcpy(m_resources.metrics_names[2], "power");
/* MONITORING START */
	mf_start(server, regplatformid, &m_resources);
}

#if defined CHAR_BIT
	// All defined OK so do nothing
#else
	#define CHAR_BIT 8
#endif

//A base-10 representation of a n-bit binary number takes up to n*log10(2) + 1 digits.
//10/33 is slightly more than log10(2). +1 
#define INT_DECIMAL_STRING_SIZE(int_type) ((CHAR_BIT*sizeof(int_type)-1)*10/33+3)

char *llint_to_string_alloc(long long int x) {
	long long int i = x;
	char buf[INT_DECIMAL_STRING_SIZE(long long int)];
	char *p = &buf[sizeof buf - 1]; // point to the end
	*p = '\0';
	if (i >= 0)
		i = -i;
	do {
		p--;
		*p = (char) ('0' - i % 10);
		i /= 10;
	} while (i);
	if (x < 0) {
		p--;
		*p = '-';
	}
	size_t len = (size_t) (&buf[sizeof buf] - p);
	char *s = (char *) malloc(len);
	if(s==NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return 0;
	}
	if (s)
		memcpy(s, p, len);
	return s;
}
 
void prepare_user_metrics(char *currentid, struct Thread_report single_thread_report ){
// 	long long int end_time=mycurrenttime();
	long long int total_execution_time = single_thread_report.end_time - single_thread_report.start_time;
	printf(" TOTAL EXECUTION TIME of thread:%9Li s ", (total_execution_time)/1000000000LL);
	long long int temp_time = (total_execution_time)%1000000000LL;
	printf(" + %3Li ms ", temp_time/1000000);
		temp_time = (total_execution_time)%1000000LL;
	printf(" + %3Li us ", temp_time/1000);
		temp_time = (total_execution_time)%1000LL;
	printf(" + %3Li ns \n", temp_time);
/* MONITORING I'd like to store here the duration of each loop --> duration */
	char metric_value[25] = {'\0'};
	sprintf(metric_value, "%Li", total_execution_time);
	const char comp_start[]="component_start";
	const char comp_end[]="component_end";
	const char duration_str[]="component_duration";
// 	const char user_defined_metric[]="user_defined_metric";
	const char run_id[]="runid";
	char *user_metrics= (char *) malloc(510);
	if(user_metrics==NULL)
		fprintf(stderr, "Failed to allocate memory.\n");
	user_metrics[0]='\0';
		// the total execution time of the component
	user_metrics=myconcat(&user_metrics,",\"", duration_str, "\":\"", metric_value,"\"");
		// common id of the different components of one execution of the task/workflow/application
	user_metrics=myconcat(&user_metrics,",\"", run_id, "\":\"", currentid,"\"");
		// name of the component
// 	user_metrics=myconcat(&user_metrics,",\"", user_defined_metric, "\":\"", user_metric_value,"\"");
		// start time of the component
	char *strstart_time =llint_to_string_alloc(single_thread_report.start_time);
	user_metrics=myconcat(&user_metrics,",\"", comp_start, "\":\"", strstart_time ,"\""); 
	free(strstart_time);
		// end time of the component
	char *strend_time =llint_to_string_alloc(single_thread_report.end_time);
	user_metrics=myconcat(&user_metrics,",\"", comp_end, "\":\"", strend_time,"\"");
	free(strend_time);
/* MONITORING END */
	mf_end();
	mf_user_metric_loc(user_metrics);
	free(user_metrics);
///* MONITORING I'd like to store here the total nr. of completed loops --> nrLoops */
//	int nrLoops=5;
//	sprintf(metric_value, "%d", nrLoops);
//	char nrLoops_str[]="nrLoops";
//	mf_user_metric_loc(nrLoops_str, metric_value);
/* MONITORING SEND */

}

void stop_monitoring(const char *server, const char *appid, const char *execfile, const char *regplatformid){
/* MONITORING END */
	char *experiment_id = mf_send(server, appid, execfile, regplatformid);
	//printf("\n> component's experiment_id is %s\n", experiment_id);
	free(experiment_id);
}

//this function query for a workflow was registered or not
//if the status code is 400 means that it was not registered before
char *query_workflow(const char *server, const char *appid){
	char* response=NULL;
	response=mf_query_workflow( server, appid ); 
	return response;
}

void register_workflow( const char *server, const char *regplatformid, const char *appid, const char *execfile){
	char author[]="new_user";
	char optimization[]="Time";
	char tasks_desc[]="[{\"device\":\"demo_desktop\", \"exec\":\"hello_world\", \"cores_nr\": \"2\"}]";
	printf("\nQUERY FOR WORKFLOW ...\n");
	char* response=query_workflow( server, appid); 
	int rc = strcmp(response, "400"); //if 400 then the workflow was not registered before
	if(rc == 0){ //0 stands for equal
		free(response);
		printf("\nREGISTERING WORKFLOW ...\n");
		response= mf_new_workflow(server, appid, author, optimization, tasks_desc);	 
	}
	free(response);
}
