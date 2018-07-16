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
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include "publisher.h"
#include "resources_monitor.h"
#include "disk_monitor.h"
#include "power_monitor.h"
#include "mf_api.h"
#include <malloc.h>

/*******************************************************************************
* Variable Declarations
******************************************************************************/
typedef struct each_metric_t {
	long sampling_interval;				//in milliseconds
	char metric_name[NAME_LENGTH];		//user defined metrics
} each_metric;

int running;
int keep_local_data_flag = 1;
char parameters_name[9][32] = {"MAX_CPU_POWER", "MIN_CPU_POWER", 
								"MEMORY_POWER", "L2CACHE_MISS_LATENCY", "L2CACHE_LINE_SIZE", 
								"E_DISK_R_PER_KB", "E_DISK_W_PER_KB", 
								"E_NET_SND_PER_KB", "E_NET_RCV_PER_KB"};
float parameters_value[9];

char DataPath[256];
pthread_t threads[MAX_NUM_METRICS];
int num_threads;
int pid;

FILE *logFile;

/*******************************************************************************
* Forward Declarations
******************************************************************************/
static int api_prepare(char *Data_path);
static void *MonitorStart(void *arg);
int get_config_parameters(const char *server, const char *platform_id);

int mf_user_metric_with_timestamp(char *user_defined_time_stamp, char *metric_name, char *value)
{
	if(DataPath[0] == '\0') {
		pid = api_prepare(DataPath);
	}
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


int mf_user_metric(char *metric_name, char *value)
{
	if(DataPath[0] == '\0') {
		pid = api_prepare(DataPath);
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

	fprintf(fp, "\"local_timestamp\":\"%.1f\", \"%s\":%s\n", timestamp_ms, metric_name, value);
	/*close the file*/
	fclose(fp);
	return 1;
}

/*
Get the pid, and setup the DataPath for data storage 
For each metric, create a thread, open a file for data storage, and start sampling the metrics periodically.
Return the path of data files
*/
struct each_metric_t **each_m=NULL;

char *mf_start(const char *server, const char *platform_id, metrics *m)
{
	/* get pid and setup the DataPath according to pid */
	pid = api_prepare(DataPath);
	/* get parameters from server with given platform_id */
	if(get_config_parameters(server, platform_id) <= 0) {
		printf("ERROR : get_config_parameters failed.\n");
		return NULL;
	}else{
// 		printf(" get_config_parameters succedd.\n");
	}
	num_threads = m->num_metrics;
	int t;
	int iret[num_threads];
	
	each_m=(struct each_metric_t **) malloc(num_threads*sizeof(struct each_metric_t *));
	for(t=0;t<num_threads;t++)
		each_m[t]=(struct each_metric_t *) malloc(1*sizeof(struct each_metric_t));
	
	running = 1;
	keep_local_data_flag = m->local_data_storage;

	for (t = 0; t < num_threads; t++) {
		/*prepare the argument for the thread*/
		each_m[t]->sampling_interval = m->sampling_interval[t];
		strcpy(each_m[t]->metric_name, m->metrics_names[t]);
		/*create the thread and pass associated arguments */
		iret[t] = pthread_create(&threads[t], NULL, MonitorStart, (each_m[t]));
		if (iret[t]) {
			printf("ERROR: pthread_create failed for %s\n", strerror(iret[t]));
			if(each_m[t]!=NULL)
				free(each_m[t]);
			each_m[t]=NULL; 
			return NULL;
		}
	} 
	return DataPath;
}

/*
Stop threads.
Close all the files for data storage
*/
void mf_end(void)
{
	int t;
	running = 0;
	for (t = 0; t < num_threads; t++) {
		pthread_join(threads[t], NULL);
	}
	int totalfree=0;
	if(each_m!=NULL){
		for (t = 0; t < num_threads; t++){
			if(each_m[t]!=NULL) 
				free(each_m[t]);
			each_m[t]=NULL;
			totalfree++;
		}
		if(totalfree==num_threads && each_m!=NULL) 
			free(each_m); 
		each_m=NULL;
	}
	printf("finished mf_end\n");
}



/*
Query for a workflow, return 400 if the workflow is not registered yet.
or 200 in other case.
*/
char* mf_query_workflow(const char *server, const char *application_id )
{
	/* create an workflow */
	char *URL = NULL;
	struct url_data response;
	response.size=0;
	response.data=NULL;
	response.headercode=NULL;

	URL=concat_and_free(&URL, server);
	URL=concat_and_free(&URL, "/v1/phantom_mf/workflows/");
	URL=concat_and_free(&URL, application_id);

// 	printf("******* register workflow ******\n");
	char operation[]="GET";
	new_query_json(URL, &response, operation); //*****
	if(URL!=NULL) free(URL);
	
	if(response.data[0] == '\0') {
		printf("ERROR: Cannot register workflow for application %s\n", application_id);
		return NULL;
	} 
	if(response.data!=NULL) free(response.data);
	return response.headercode;
}



/*
Register a new workflow.
Return the path to query the workflow.
*/
char* mf_new_workflow(const char *server, const char *application_id, const char *author_id,
		const char *optimization, const char *tasks_desc)
{
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

// 	printf("******* register workflow ******\n"); 
	char operation[]="PUT";
	query_message_json(URL, msg, &response, operation); //***** 
	
	if(msg!=NULL) free(msg);
	if(URL!=NULL) free(URL);
	if(response.headercode!=NULL) free(response.headercode);
	if(response.data[0] == '\0') {
		printf("ERROR: Cannot register workflow for application %s\n", application_id);
		return NULL;
	}
	return response.data;
}


/*
Generate the execution_id.
Send the monitoring data in all the files to mf_server.
Return the execution_id
*/
char* mf_send(const char *server, const char *application_id, const char *component_id, const char *platform_id)
{
	/* create an experiment with regards of given application_id, component_id and so on */
	char *URL = NULL;
	struct url_data response;
	response.size=0;
	response.data=NULL;
	response.headercode=NULL;
// 	char *experiment_id = NULL;
	char *msg = NULL;
	msg=concat_and_free(&msg, "{\"application\":\"");
	msg=concat_and_free(&msg, application_id);
	msg=concat_and_free(&msg, "\", \"task\": \"");
	msg=concat_and_free(&msg, component_id);
	msg=concat_and_free(&msg, "\", \"host\": \"");
	msg=concat_and_free(&msg, platform_id);
	msg=concat_and_free(&msg, "\"}");

	URL=concat_and_free(&URL, server);
	URL=concat_and_free(&URL, "/v1/phantom_mf/experiments/");
	URL=concat_and_free(&URL, application_id);
	
	
	
// 	printf("******* new_create_new_experiment ******\n"); 
	char operation[]="POST";
	query_message_json(URL, msg, &response, operation); //***** 
	
	if(msg!=NULL) free(msg);
	if(URL!=NULL) free(URL);
	if(response.headercode!=NULL) free(response.headercode);
	if(response.data[0] == '\0') {//response->data containes the experiment_id
		printf("ERROR: Cannot create new experiment for application %s\n", application_id);
		return NULL;
	} 

	/*malloc variables for send metrics */
	char *metric_URL = (char *) malloc(200);
	char *static_string = (char *) malloc(200);
	char *filename = (char *) malloc(200);
	metric_URL[0]='\0';
	metric_URL=concat_and_free(&metric_URL, server);
	metric_URL=concat_and_free(&metric_URL, "/v1/phantom_mf/metrics");

	DIR *dir = opendir(DataPath);
	if(dir == NULL) {
		printf("Error: Cannot open directory %s\n", DataPath);
		return NULL;
	}

	struct dirent *drp = readdir(dir);
	while(drp != NULL) { 
		static_string[0]='\0';//in the next loop the string will be empty
		//memset(static_string, '\0', malloc_usable_size(static_string));//this is too much, not need that effort
		filename[0]='\0';//in the next loop the string will be empty
		//memset(filename, '\0', malloc_usable_size(filename)); //this is too much, not need that effort
		if(drp->d_name[0]!='.'){//not wish to process (. .. or hidden files)
			filename=concat_and_free(&filename, DataPath);
			filename=concat_and_free(&filename, "/");
			filename=concat_and_free(&filename, drp->d_name);

			//static_string=concat_and_free(&static_string, "{");//****
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
			//static_string=concat_and_free(&static_string, "}");
			
			publish_file(metric_URL, static_string, filename);
			/*remove the file if user unset keep_local_data_flag */
			if(keep_local_data_flag == 0) {
				unlink(filename);
			}
		}
			
		/*get the next entry */
		drp = readdir(dir);
	}

	
	if(metric_URL!= NULL)
		free(metric_URL);
	if(static_string!= NULL)
		free(static_string);
	if(filename!= NULL)
		free(filename);
	closedir(dir);
	fclose(logFile);

	/*remove the data directory if user unset keep_local_data_flag */
	if(keep_local_data_flag == 0) {
		rmdir(DataPath);
	}
	return response.data; // it contains the experiment_id;
}

/*
Get the pid, and setup the DataPath for data storage 
*/
static int api_prepare(char *Data_path)
{
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
	sprintf(logFileName, "%s/log.txt", Data_path);
	logFile = fopen(logFileName, "w");
	if (logFile == NULL) {
		printf("Could not create log file %s", logFileName);
	}

	/*create the folder with regards of the pid*/
	sprintf(Data_path + strlen(Data_path), "/%d", pid);
	struct stat st = { 0 };
	if (stat(Data_path, &st) == -1)
		mkdir(Data_path, 0700);
	return pid;
}


static void *MonitorStart(void *arg) { 
	each_metric *metric = (each_metric*) arg;
	if(strcmp(metric->metric_name, METRIC_NAME_1) == 0) {
		resources_monitor(pid, DataPath, metric->sampling_interval);
	} else if(strcmp(metric->metric_name, METRIC_NAME_2) == 0) {
		disk_monitor(pid, DataPath, metric->sampling_interval);
	} else if(strcmp(metric->metric_name, METRIC_NAME_3) == 0) {
		power_monitor(pid, DataPath, metric->sampling_interval);
	} else {
		printf("ERROR: it is not possible to monitor %s\n", metric->metric_name); 
	} 
	return NULL;
}


/**
 * Returns 1 if succedd otherwise returns 0
 */
int get_config_parameters(const char *server, const char *platform_id)
{
	/* send the query and retrieve the response string */
	char *URL =(char *) malloc(1024);
	struct url_data response ; //it is reserved by new_query_json
	char operation[]="GET";
	sprintf(URL, "%s/v1/phantom_rm/configs/%s", server, platform_id);

	if(new_query_json(URL, &response, operation) <= 0) {
		printf("ERROR: query with %s failed.\n", URL);
		if(URL!=NULL) free(URL);
		if(response.data!=NULL) free(response.data);
		if(response.headercode!=NULL) free(response.headercode);
		return 0;
	}
	if(URL!=NULL) free(URL);
	if(strstr(response.data, "parameters") == NULL) {
		printf("ERROR: response does not include parameters.\n");
		if(response.data!=NULL) free(response.data);
		if(response.headercode!=NULL) free(response.headercode);
		return 0;
	}

	/* parse the send back string to get required parameters */
	char *ptr_begin, *ptr_end;
	char value[16] = {'\0'};
	int i, value_length;
	for (i = 0; i <= 8; i++) {
		ptr_begin = strstr(response.data, parameters_name[i]);
		if(ptr_begin != NULL) {
			ptr_end = strstr(ptr_begin, ",");
			if(ptr_end == NULL) {
				ptr_end = strstr(ptr_begin, "}");
				if(ptr_end == NULL){
					if(response.data!=NULL) free(response.data);
					if(response.headercode!=NULL) free(response.headercode);
					return 0;
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
	if(response.data!=NULL) free(response.data);
	if(response.headercode!=NULL) free(response.headercode);
	return 1;
}


/**
 * NEW FUNCTIONS DEVELOPED DURING THE INTEGRARION
 */

// #include <unistd.h>
// #include <sys/stat.h>
// #include <string.h> 
// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>

// #include <malloc.h>
#include "mf_api.h"
// #include <sys/time.h>

// int pid;
// char DataPath[256];


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


// //Function for increase dynamically a string concatenating strings at the end
// //It free the memory of the first pointer if not null
// char* concat_and_free(char **s1, const char *s2){
// 	char *result = NULL;
// 	unsigned int new_lenght= strlen(s2)+1; //+1 for the null-terminator;
// 	if(*s1 != NULL){
// 		new_lenght+= strlen(*s1);//current lenght
// 		if(new_lenght> malloc_usable_size(*s1)){
// 			result = (char *) malloc(new_lenght);
// 			strcpy(result, *s1);
// 			free(*s1);
// 		}else{
// 			result = *s1;
// 		} 
// 	}else{
// 		result = (char *) malloc(new_lenght);
// 		result[0]='\0';
// 	}
// 	//in real code you would check for errors in malloc here 
// 	strcat(result, s2);
// 	return result;
// }

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


void user_metrics_buffer(
char *currentid, struct Thread_report single_thread_report ){ 
	unsigned int j;
/* MONITORING I'd like to store here the duration of each loop --> duration */  
	const char component_name[]="component_name";
	const char run_id[]="runid"; 
	char *user_metrics= (char *) malloc(510);
	if(user_metrics==NULL)
		fprintf(stderr, "Failed to allocate memory.\n");
for(j=0;j<single_thread_report.total_metrics;j++)	{
	user_metrics[0]='\0';
		// common id of the different components of one execution of the task/workflow/application
	user_metrics=myconcat(&user_metrics,",\"", run_id, "\":\"", currentid,"\"");
	user_metrics=myconcat(&user_metrics,",\"", component_name, "\":\"", single_thread_report.taskid,"\"");
	user_metrics=myconcat(&user_metrics,",\"", "metric_time", "\":\"", single_thread_report.metric_time[j],"\""); 
	user_metrics=myconcat(&user_metrics,",\"", single_thread_report.user_label[j], "\":\"",   single_thread_report.user_value[j],"\"");

/* MONITORING END */
	mf_user_metric_loc(user_metrics);
}
	free(user_metrics);
///* MONITORING I'd like to store here the total nr. of completed loops --> nrLoops */
//	int nrLoops=5;
//	sprintf(metric_value, "%d", nrLoops);
//	char nrLoops_str[]="nrLoops";
//	mf_user_metric_loc(nrLoops_str, metric_value);
/* MONITORING SEND */
}


void register_end_component( 
char *currentid, struct Thread_report single_thread_report ){
	long long int total_execution_time = single_thread_report.end_time - single_thread_report.start_time;
// 	printf(" TOTAL EXECUTION TIME:%9Li s ", (total_execution_time)/1000000000LL);
// 	long long int temp_time = (total_execution_time)%1000000000LL;
// 	printf(" + %3Li ms ", temp_time/1000000);
// 		temp_time = (total_execution_time)%1000000LL;
// 	printf(" + %3Li us ", temp_time/1000);
// 		temp_time = (total_execution_time)%1000LL;
// 	printf(" + %3Li ns \n", temp_time);
/* MONITORING I'd like to store here the duration of each loop --> duration */
	char *metric_value  ;
// 	sprintf(metric_value, "%Li", total_execution_time);
	metric_value=llint_to_string_alloc(total_execution_time);
	const char comp_start[]="component_start";
	const char comp_end[]="component_end";
	const char duration_str[]="component_duration"; 
// 	const char user_defined_metric[]="user_defined_metric";
// 	const char run_id[]="runid";
	
	const char component_name[]="component_name";
	const char run_id[]="runid";
	
	char *user_metrics= (char *) malloc(510);
	if(user_metrics==NULL)
		fprintf(stderr, "Failed to allocate memory.\n");
	user_metrics[0]='\0';
		// the total execution time of the component
	user_metrics=myconcat(&user_metrics,",\"", duration_str, "\":\"", metric_value,"\"");
		// common id of the different components of one execution of the task/workflow/application
	user_metrics=myconcat(&user_metrics,",\"", run_id, "\":\"", currentid,"\"");
	char *strstart_time =llint_to_string_alloc(single_thread_report.start_time);
	user_metrics=myconcat(&user_metrics,",\"", comp_start, "\":\"", strstart_time ,"\""); 
	free(strstart_time);
		// end time of the component
	char *strend_time =llint_to_string_alloc(single_thread_report.end_time);
	user_metrics=myconcat(&user_metrics,",\"", comp_end, "\":\"", strend_time,"\"");
	free(strend_time);
	free(metric_value);
/* MONITORING END */
	mf_user_metric_loc(user_metrics);
	free(user_metrics);
///* MONITORING I'd like to store here the total nr. of completed loops --> nrLoops */
//	int nrLoops=5;
//	sprintf(metric_value, "%d", nrLoops);
//	char nrLoops_str[]="nrLoops";
//	mf_user_metric_loc(nrLoops_str, metric_value);
/* MONITORING SEND */
}
 

void monitoring_send(const char *server, const char *appid,const char *execfile, const char *regplatformid){
	/* MONITORING SEND */
	char *experiment_id = mf_send(server, appid, execfile, regplatformid);
// 	printf(" experiment_id is %s\n",experiment_id);
	if(experiment_id!=NULL) free(experiment_id);//dinamically allocated by mf_send 
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
// 	printf("\nQUERY FOR WORKFLOW ...\n");
	char* response=query_workflow( server, appid); 
	int rc = strcmp(response, "400"); //if 400 then the workflow was not registered before
	if(rc == 0){ //0 stands for equal
		free(response);
// 		printf("\nREGISTERING WORKFLOW ...\n");
		response= mf_new_workflow(server, appid, author, optimization, tasks_desc);	 
	}
	free(response);
}


