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
	long long int timeus = (long int) (ts_start.tv_sec*1000000000LL + ts_start.tv_nsec);
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
int get_config_parameters(const char *server,const char *platform_id,const char *token);

/*******************************************************************************
* Function Definitions
******************************************************************************/

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

	fprintf(fp, "\"local_timestamp\":\"%.1f\", \"%s\":%s\n", timestamp_ms, metric_name, value);
	/*close the file*/
	fclose(fp);
	return 1;
}

/**
* Get the pid, and setup the DataPath for data storage.
* For each metric, create a thread, open a file for data storage, and start sampling the metrics periodically.
* @return the path of data files
*/
struct each_metric_t **each_m=NULL;

/** server consists on an address or ip with a port number like http://129.168.0.1:8600/ */
char *mf_start(const char *server,const char *platform_id, metrics *m,const char *token) {
	/* get pid and setup the DataPath according to pid */
	pid = api_prepare(DataPath);
	/* get parameters from server with given platform_id */
	if(get_config_parameters(server, platform_id, token) != SUCCESS) {
		printf("ERROR : get_config_parameters failed.\n");
		return NULL;
	}
	//printf(" get_config_parameters succeed.\n");
	printf("num_threads %i\n",m->num_metrics);
	fflush(stdout);
	num_threads = m->num_metrics;
	int t;
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

/**
* Stop threads.
* Close all the files for data storage.
*/
void mf_end(void){
	int t;
	running = 0;
	for (t = 0; t < num_threads; t++)
		pthread_join(threads[t], NULL);
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
	close_curl();
	printf("finished mf_end\n");
}

/**
* Query for a workflow, return 400 if the workflow is not registered yet.
* or 200 in other case.
* @return NULL if error
*/
char* mf_query_workflow(char *server, char *application_id ){
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
	new_query_json(URL, &response, operation,NULL); //*****
	
	if(new_query_json(URL, &response, operation, NULL) > 0) {
		printf("ERROR: query with %s failed.\n", URL);
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
* @return  the path to query the workflow.
*/
char* mf_new_workflow(char *server, char *application_id, char *author_id,
		char *optimization, char *tasks_desc, char *token) {
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
	query_message_json(URL, msg, &response, operation, token); //***** 
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
	return response.data;
}


/**
* Generate the execution_id.
* Send the monitoring data in all the files to mf_server.
*@return the execution_id
*/
char* mf_send(const char *server,const  char *application_id,const  char *component_id,const  char *platform_id,const  char *token) {
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
	if(query_message_json(URL, msg, &response, operation, token)==FAILED){
		printf("ERROR: Cannot create new experiment for application %s\n", application_id);
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
// printf(" processing file %s\n",filename);
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
			publish_file(metric_URL, static_string, filename, token);
			/*remove the file if user unset keep_local_data_flag */
			if(keep_local_data_flag == 0)
				unlink(filename);
		}
		/*get the next entry */
		drp = readdir(dir);
	}

	if(metric_URL!= NULL)
		free(metric_URL);
	metric_URL=NULL;
	if(static_string!= NULL)
		free(static_string);
	static_string=NULL;
	if(filename!= NULL)
		free(filename);
	filename=NULL;
	closedir(dir);
	if(logFile != NULL) 
		fclose(logFile); 
	logFile=NULL;

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
// 	printf("Starting monitoring modules\n");fflush(stdout);
	if(strcmp(metric->metric_name, METRIC_NAME_1) == 0) {
// 		printf("Starting monitoring modul %s\n",METRIC_NAME_1);fflush(stdout);
		linux_resources(pid, DataPath, metric->sampling_interval);
	} else if(strcmp(metric->metric_name, METRIC_NAME_2) == 0) {
// 		printf("Starting monitoring modul %s\n",METRIC_NAME_2);fflush(stdout);
		xlnx_monitor(DataPath, metric->sampling_interval);
	} else if(strcmp(metric->metric_name, METRIC_NAME_3) == 0) {
// 		printf("Starting monitoring modul %s\n",METRIC_NAME_3);fflush(stdout);
		power_monitor(pid, DataPath, metric->sampling_interval);
#ifdef NVML
#if NVML == yes
	} else if(strcmp(metric->metric_name, METRIC_NAME_4) == 0) {
// 		printf("Starting monitoring modul %s\n",METRIC_NAME_4);fflush(stdout);
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

void debugging_mf_configs(struct json_mf_config **mf_config,const unsigned int total_loaded_mf_configs ){
// 	print_counters_from_json(html);
// 	print_counters_from_parsed_json(mf_config, total_loaded_mf_configs);
// 	print_json(html);
	print_parsed_json(mf_config,total_loaded_mf_configs);
}


/**
* @return SUCCESS(0) if succeed, otherwise returns FAILED
*/
int get_config_parameters(const char *server,const char *platform_id,const char *token){
	/* send the query and retrieve the response string */
// 	char resoucemanager_path[]="query_device_mf_config?pretty=true\\&device=\"";//pretty is not needed
	if(strlen(platform_id)==0)
		return 1;//we can not proceed without a platform_id
		
	unsigned int total_loaded_mf_configs =0;
	struct json_mf_config **mf_config=NULL;
	const char *resource_server="141.58.0.8:2780";//examples: 141.58.0.8:2780 or localhost:8600
	
	char *html=query_mf_config(resource_server, platform_id, token);
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
	int num,counter, total_fields,total_objects;
	for(num =0; num <total_loaded_mf_configs;num++){
		for(total_fields =0;  total_fields<mf_config[num]->count_f; total_fields++){
			total_objects= mf_config[num]->field[total_fields]->count_o;
			for(counter=0;counter< total_objects ;counter++){
				free(mf_config[num]->field[total_fields]-> object[counter]-> label_o);
				free(mf_config[num]->field[total_fields]-> object[counter]-> value_o);
				free(mf_config[num]->field[total_fields]-> object[counter]);
			}
			free(mf_config[num]->field[total_fields]->object);
			free(mf_config[num]->field[total_fields]->label_f);
			free(mf_config[num]->field[total_fields]);
		}
		free(mf_config[num]->field);
		free(mf_config[num]);
	}
	free(mf_config);
	
// 	unsigned int need_string_size=strlen(server)+ strlen(resoucemanager_path)+ strlen(platform_id)+ strlen("\"")+1;
// 	if (server[strlen(server)] != '/' )
// 		need_string_size++;// we need to add an slash "/"
// 
// 	char *URL =(char *) malloc(need_string_size);
// 	if(URL==NULL) {
// 		printf("Failed to allocate memory.\n");
// 		exit(1);
// 	}
// 	strcpy(URL,server);
// 	if (server[strlen(server)] != '/' )
// 		strcat(URL,"/"); // we need to add an slash "/"
// 	strcat(URL,resoucemanager_path);
// 	strcat(URL,platform_id);
// 	strcat(URL,"\"");
// 
// 	struct url_data response; //it is reserved by new_query_json, new_query_json is defined in publisher.c
// 	response.data=NULL;
// 	response.headercode = NULL;
// 	char operation[]="GET";
// 
// 	int mycode =new_query_json(URL, &response, operation,token);
// 	if(mycode > 0) {
// 		printf("ERROR: query with %s failed code %i.\n", URL, mycode);
// 		if(URL!=NULL) free(URL);
// 		URL=NULL;
// 		if(response.data!=NULL) { free(response.data); response.data=NULL; }
// 		if(response.headercode!=NULL) { free(response.headercode); response.headercode = NULL; }
// 		return FAILED;
// 	}

// 	if(strstr(response.data, "parameters") == NULL) {
// 		printf("ERROR: response does not include parameters.\n");
// 		printf("response.data: %s.\n",response.data);
// 		printf("GET URL: %s.\n",URL);
// 		if(URL!=NULL) { free(URL); URL=NULL; }
// 		if(response.data!=NULL) { free(response.data); response.data=NULL; }
// 		if(response.headercode!=NULL) { free(response.headercode); response.headercode = NULL; }
// 		return FAILED;
// 	}
// 	if(URL!=NULL) { free(URL); URL=NULL; }
	/* parse the send back string to get required parameters */

/*
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
	if(user_query->query!=NULL) free(user_query->query);
	if(user_query!=NULL) free(user_query);
	user_query=NULL;
}


void submit_metric_json(char *user_string){
	printf("%s\n\n",user_string);
}

/**
* Get the pid, and setup the DataPath for data storage
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

int mf_user_metric_loc_type(char *user_metrics, const char *datatype){
	if(DataPath[0] == '\0')
		pid = api_prepare_loc(DataPath);
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


void user_metrics_buffer(char *currentid, struct Thread_report single_thread_report ){
	unsigned int j;
/* MONITORING I'd like to store here the duration of each loop --> duration */
	char component_name[]="component_name";
	char run_id[]="runid";
	char *user_metrics= (char *) malloc(510);
	if(user_metrics==NULL)
		fprintf(stderr, "Failed to allocate memory.\n");
for(j=0;j<single_thread_report.total_metrics;j++){
	user_metrics[0]='\0';
		// common id of the different components of one execution of the task/workflow/application
	user_metrics=myconcat(&user_metrics,",\"", run_id, "\":\"", currentid,"\"");
	user_metrics=myconcat(&user_metrics,",\"", component_name, "\":\"", single_thread_report.taskid,"\"");
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


void register_end_component(char *currentid, struct Thread_report single_thread_report ){
	long long int total_execution_time = single_thread_report.end_time - single_thread_report.start_time;
// 	printf(" TOTAL EXECUTION TIME:%9Li s ", (total_execution_time)/1000000000LL);
// 	long long int temp_time = (total_execution_time)%1000000000LL;
// 	printf(" + %3Li ms ", temp_time/1000000);
// 		temp_time = (total_execution_time)%1000000LL;
// 	printf(" + %3Li us ", temp_time/1000);
// 		temp_time = (total_execution_time)%1000LL;
// 	printf(" + %3Li ns \n", temp_time);
/* MONITORING I'd like to store here the duration of each loop --> duration */
	char metric_value[100];
	// 	sprintf(metric_value, "%Li", total_execution_time);
	llint_to_string_alloc(total_execution_time,metric_value);
	char comp_start[]="component_start";
	char comp_end[]="component_end";
	char duration_str[]="component_duration";
	char taskid_str[]="component_name";
// 	char user_defined_metric[]="user_defined_metric";
// 	char run_id[]="runid";

// 	char component_name[]="component_name";
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
	user_metrics=myconcat(&user_metrics,",\"", comp_start, "\":\"", strstart_time ,"\"");
		// end time of the component
	char strend_time[100];
	llint_to_string_alloc(single_thread_report.end_time,strend_time);
	user_metrics=myconcat(&user_metrics,",\"", comp_end, "\":\"", strend_time,"\"");

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


void monitoring_send(char *server, char *appid,char *execfile, char *regplatformid, char *token){
	/* MONITORING SEND */
	char *experiment_id = mf_send(server, appid, execfile, regplatformid, token);
// 	printf(" experiment_id is %s\n",experiment_id);
	if(experiment_id!=NULL) free(experiment_id);//dinamically allocated by mf_send 
	experiment_id=NULL;
}


/**
* this function query for a workflow was registered or not
* if the status code is 400 means that it was not registered before*/
char *query_workflow(char *server, char *appid){
	char* response=NULL;
	response=mf_query_workflow( server, appid );
	return response;
}

/** @return 0 if success*/
int register_workflow( char *server, char *regplatformid, char *appid, char *execfile, char *token){
	char author[]="new_user";
	char optimization[]="Time";
	char tasks_desc[]="[{\"device\":\"demo_desktop\", \"exec\":\"hello_world\", \"cores_nr\": \"2\"}]";
// 	printf("\nQUERY FOR WORKFLOW ...\n");
	char* response=query_workflow( server, appid);
	if (response==NULL) {
		printf("ERROR registering workflow server: %s\n",server);
		return 1;
	}
	int rc = strcmp(response, "400"); //if 400 then the workflow was not registered before
	if(rc == 0){ //0 stands for equal
		if(response!=NULL) free(response);
// 		printf("\nREGISTERING WORKFLOW ...\n");
		response= mf_new_workflow(server, appid, author, optimization, tasks_desc, token);
	}
	if(response!=NULL) free(response);
	return 0;
}
