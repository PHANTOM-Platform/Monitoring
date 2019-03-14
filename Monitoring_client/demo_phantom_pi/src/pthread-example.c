#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "mf_util.h"
#include "mf_api.h"

#define SUCCESS 0
#define FAILURE 1

#define total_num_threads 2
pthread_t tid[total_num_threads];
/*******************************************************************************
 * External Variables Declarations
 ******************************************************************************/
FILE *logFile;		//declared as extern in mf_debug.h
/*******************************************************************************
 * Variable Declarations
 ******************************************************************************/
int pwd_is_set = 0;
char currentid[100]; //currentid: common UNIQUE id for register in the MF all the threads-metrics in this execution.

void my_start_monitoring(const char *mf_server, const char *exec_manager, const char *exec_id, const char* resource_manager, const char *platform_id, const char *token){
/* MONITORING METRICS: optional definition, this will merge over the default one loaded from the Resource Manager */
	metrics m_resources;
	m_resources.num_metrics = 3;
	if (m_resources.num_metrics>MAX_NUM_METRICS){
		printf("ERROR: the maximum amount of plugin metrics is %i\n",MAX_NUM_METRICS);
		exit(1);
	}
	m_resources.local_data_storage = 1; /*remove the file if user unset keep_local_data_flag */
	m_resources.sampling_interval[0] = 10; // 1000 stands for 1s
	strcpy(m_resources.metrics_names[0], "resources_usage");
	m_resources.sampling_interval[1] = 10; // ms
	strcpy(m_resources.metrics_names[1], "power");
	m_resources.sampling_interval[2] = 10; // ms
	strcpy(m_resources.metrics_names[2], "xlnx_monitor");
	/******************** Initialize of some registers ************************/
	struct app_report_t *my_app_report= reserve_app_report(total_num_threads, currentid);//argv[1] is a text string for later friendly query the results
	strcpy(my_app_report->my_thread_report[0]->taskid,"component_a");
	strcpy(my_app_report->my_thread_report[1]->taskid,"component_b");
	/********************MONITORNG START ************************/
/* MONITORING START */
	mf_start(mf_server, exec_manager, exec_id, resource_manager, platform_id, &m_resources,my_app_report,token);//returns char *datapath
}

int requested_mon =0;
//this is the code of the threads
void* doSomeThing(void *args){
//Initialization of the struct for storing the monitoring-metrics and user-metrics of the component
	const unsigned int total_metrics =10;
	struct timespec timestamp;
	struct Thread_report_t *my_thread_report = (struct Thread_report_t *)args;
	my_thread_report->start_time=mycurrenttime();  // in ns but accuracy is about tens of micro-seconds (us)
	clock_gettime(CLOCK_REALTIME, &timestamp);
	/*convert to milliseconds */
	my_thread_report->start_timestamp_ms = timestamp.tv_sec * 1000.0 + (double)(timestamp.tv_nsec / 1.0e6);// in ms + one decimal position
// 	reserve_memory_user_def_metrics(my_thread_report, total_metrics);//this reserves the memory for user_defined_metrics
//run some code, and collect some user metrics
	char mytime[100];
	char* buffer =NULL;
	pthread_t id = pthread_self();

		pid_t tidb = syscall(SYS_gettid);
// 		pid_t tidb = syscall(__NR_gettid);
		pid_t pidb =getpid();
		printf(" PID %u TID %u\n", pidb, tidb);
		add_tid_to_report(my_thread_report->taskid,tidb);

	for (int i=0;i<total_metrics;i++){
		llint_to_string_alloc(mycurrenttime(),mytime);// <<-- TIME
		printf("\n Processing thread named as: %s\n",my_thread_report->taskid);
		if(pthread_equal(id,tid[0])) {
// 			strcpy(my_thread_report->user_label[0], "n_ships_found"); // <<-- LABEL
// 			itoa((int) 10*i+5, my_thread_report->user_value[0]);      // <<-- VALUE
			register_user_metric( my_thread_report, mytime, "n_ships_found", 10*i+5, 0);
		} else {
// 			strcpy(my_thread_report->user_label[0], "number_of_blocks"); // <<-- LABEL
// 			itoa((int) 20*i+8, my_thread_report->user_value[0]);         // <<-- VALUE
			register_user_metric( my_thread_report, mytime, "number_of_blocks", 20*i+8, 0);
		}
		register_user_metric( my_thread_report, mytime, "counter", i, 1);
// 		strcpy(my_thread_report->metric_time[0], mytime);   // <<-- TIME
// 		strcpy(my_thread_report->user_label[1], "counter"); // <<-- LABEL
// 		itoa((int) i, my_thread_report->user_value[1]);     // <<-- VALUE
// 		strcpy(my_thread_report->metric_time[1], mytime);   // <<-- TIME
		user_metrics_buffer(*my_thread_report);
		usleep(1000000);  /* sleep for 1000 milliSeconds */
	}
//end of the thread or component
	printf("\n Finishing thread named as: %s\n",my_thread_report->taskid);
	fflush(stdout);
	if(pthread_equal(id,tid[0]))
		free(buffer);
	my_thread_report->end_time=mycurrenttime();
    return NULL;
}

int main(int argc, char* argv[]) {
	/* init arguments */
	char mf_server[]="localhost:3033";// 141.58.0.8
	char exec_manager[]="localhost:8700";// 141.58.0.8
	char resource_manager[]="localhost:8600";// 141.58.0.8	
	char appid[]="montanana_demo";
	char regplatformid[]="node01";
	char execfile[]="pthread-example";
	char exec_id[]="1234";	
	char token[]="eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJtb250YW5hQGFiYy5jb20iLCJpYXQiOjE1NDQwODg1OTksImV4cCI6MTU0NjY4MDU5OX0.XvDSVyz89S3segsIUrH60e7FcI6i0W_ApPqcW-5bXn8";
	/******************** Initialize of some registers ************************/
	char currentid[100];
	strcpy(currentid,argc>1?argv[1]:"missingid");
	currentid[99]='\0'; //for protecting from run out of reserved memory
	struct app_report_t *my_app_report= reserve_app_report(total_num_threads, currentid);//argv[1] is a text string for later friendly query the results
	strcpy(my_app_report->my_thread_report[0]->taskid,"component_a");
	strcpy(my_app_report->my_thread_report[1]->taskid,"component_b");
	/********************MONITORNG START ************************/
	my_start_monitoring(mf_server, exec_manager, exec_id, resource_manager, regplatformid, token);
	/********************* THE APPLICATION ***********************************/
	printf("\n Starting program\n");
	for(int i=0;i<total_num_threads;i++){
		int err = pthread_create(&(tid[i]), NULL, &doSomeThing, (void *) (my_app_report->my_thread_report[i]));
		if (err != 0)
			printf("\n Can't create thread :[%s]", strerror(err));
		else
			printf("\n Thread created successfully\n");
	}
	for(int i=0;i<total_num_threads;i++)
		(void) pthread_join(tid[i], NULL);
	printf("\n Finishing program\n");
	/************* MONITORING END *******************************/
	if (register_workflow(mf_server, regplatformid, appid, execfile,token)!=SUCCESS){
		printf("Error registering workflow on server %s\n",mf_server);fflush(stdout);
	} else{
		printf("Success registering workflow on server %s\n",mf_server);fflush(stdout);
	};
	for(int i=0;i<total_num_threads;i++)
		register_end_component(my_app_report->currentid, *my_app_report->my_thread_report[i]);
	//monitoring_send can be call at any time, but must be call after mf_end, to forward the last remaining stats
	monitoring_end(mf_server, exec_manager, appid, exec_id, execfile, regplatformid,token, my_app_report);//it calls the mf_end and mf_send and frees memory, and closes logfile
    return 0;
}
