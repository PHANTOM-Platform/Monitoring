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
int get_config_parameters(char *server, char *platform_id);

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
	double timestamp_ms = timestamp.tv_sec * 1000.0  + (double)(timestamp.tv_nsec / 1.0e6);

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
char *mf_start(char *server, char *platform_id, metrics *m)
{
	/* get pid and setup the DataPath according to pid */
    pid = api_prepare(DataPath);

    /* get parameters from server with given platform_id */
    if(get_config_parameters(server, platform_id) <= 0) {
    	printf("ERROR : get_config_parameters failed.\n");
    	return NULL;
    }

	num_threads = m->num_metrics;
	int t;
	int iret[num_threads];
	each_metric *each_m = malloc(num_threads * sizeof(each_metric));

	running = 1;
	keep_local_data_flag = m->local_data_storage;

	for (t = 0; t < num_threads; t++) {
		/*prepare the argument for the thread*/
		each_m[t].sampling_interval = m->sampling_interval[t];
		strcpy(each_m[t].metric_name, m->metrics_names[t]);
		/*create the thread and pass associated arguments */
		iret[t] = pthread_create(&threads[t], NULL, MonitorStart, &(each_m[t]));
		if (iret[t]) {
			printf("ERROR: pthread_create failed for %s\n", strerror(iret[t]));
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
}

/*
Generate the execution_id.
Send the monitoring data in all the files to mf_server.
Return the execution_id
*/
char *mf_send(char *server, char *application_id, char *component_id, char *platform_id)
{
	/* create an experiment with regards of given application_id, component_id and so on */
	char *msg = calloc(256, sizeof(char));
	char *URL = calloc(256, sizeof(char));
	char *experiment_id = calloc(64, sizeof(char));
	
	sprintf(msg, "{\"application\":\"%s\", \"task\": \"%s\", \"host\": \"%s\"}",
		application_id, component_id, platform_id);
	sprintf(URL, "%s/v1/phantom_mf/experiments/%s", server, application_id);

	create_new_experiment(URL, msg, experiment_id);
	if(experiment_id[0] == '\0') {
		printf("ERROR: Cannot create new experiment for application %s\n", application_id);
		return NULL;
	}
	//sleep(5);

	/*malloc variables for send metrics */
	char *metric_URL = calloc(256, sizeof(char));
	char *static_string = calloc(256, sizeof(char));
	char *filename = calloc(256, sizeof(char));

	sprintf(metric_URL, "%s/v1/phantom_mf/metrics", server);

	DIR *dir = opendir(DataPath);
	if(dir == NULL) {
		printf("Error: Cannot open directory %s\n", DataPath);
		return NULL;
	}

	struct dirent *drp = readdir(dir);

	while(drp != NULL) {

		sprintf(filename, "%s/%s", DataPath, drp->d_name);
		sprintf(static_string, "\"WorkflowID\":\"%s\", \"TaskID\":\"%s\", \"ExperimentID\":\"%s\", \"type\":\"%s\", \"host\":\"%s\"",
			application_id, component_id, experiment_id, drp->d_name, platform_id);
		
		publish_file(metric_URL, static_string, filename);

		/*remove the file if user unset keep_local_data_flag */
		if(keep_local_data_flag == 0) {
			unlink(filename);
		}

		/*get the next entry */
		drp = readdir(dir);
		memset(static_string, '\0', 256);	
		memset(filename, '\0', 256);	
	}

	closedir(dir);
	fclose(logFile);

	/*remove the data directory if user unset keep_local_data_flag */
	if(keep_local_data_flag == 0) {
		rmdir(DataPath);
	}
	return experiment_id;
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
	}
	else if(strcmp(metric->metric_name, METRIC_NAME_2) == 0) {
		disk_monitor(pid, DataPath, metric->sampling_interval);
	}
	else if(strcmp(metric->metric_name, METRIC_NAME_3) == 0) {
		power_monitor(pid, DataPath, metric->sampling_interval);
	}
	else {
		printf("ERROR: it is not possible to monitor %s\n", metric->metric_name);
		return NULL;
	}
	return NULL;
}

int get_config_parameters(char *server, char *platform_id)
{
	/* send the query and retrieve the response string */
	char *URL = calloc(256, sizeof(char));
	char *response_str = calloc(256, sizeof(char));

	sprintf(URL, "%s/v1/phantom_rm/configs/%s", server, platform_id);
	if(query_json(URL, response_str) <= 0) {
		printf("ERROR: query with %s failed.\n", URL);
		return 0;
	}
	if(strstr(response_str, "parameters") == NULL) {
		printf("ERROR: response does not include parameters.\n");
		return 0;
	}

	/* parse the send back string to get required parameters */
	char *ptr_begin, *ptr_end;
	char value[16] = {'\0'};
	int i, value_length;
	for (i = 0; i <= 8; i++) {
		ptr_begin = strstr(response_str, parameters_name[i]);
		if(ptr_begin != NULL) {
			ptr_end = strstr(ptr_begin, ",");
			if(ptr_end == NULL) {
				ptr_end = strstr(ptr_begin, "}");
				if(ptr_end == NULL)
					return 0;
			}
			value_length = ptr_end - ptr_begin - 4 - strlen(parameters_name[i]);
			ptr_begin += 3 + strlen(parameters_name[i]);
			memset(value, '\0', 16);
			strncpy(value, ptr_begin, value_length);
			parameters_value[i] = atof(value);
		}
	}
	/*
	printf("parameters are:\n");
	for (i = 0; i <= 8; i++) {
		printf("%s:%f\n", parameters_name[i], parameters_value[i]);
	}
	*/
	return 1;
}