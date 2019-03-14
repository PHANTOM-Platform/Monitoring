#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <gmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "pi_code.h"
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

workInfo_t workInfo;

//this is the code of the threads
void *doThread(void *args) {
//Initialization of the struct for storing the monitoring-metrics and user-metrics of the component
// 	const unsigned int total_user_metrics =2;
	struct timespec timestamp;
	struct Thread_report_t *my_thread_report = (struct Thread_report_t *)args;
	my_thread_report->start_time=mycurrenttime();  // in ns but accuracy is about tens of micro-seconds (us)
	clock_gettime(CLOCK_REALTIME, &timestamp);
	/*convert to milliseconds */
	my_thread_report->start_timestamp_ms = timestamp.tv_sec * 1000.0 + (double)(timestamp.tv_nsec / 1.0e6);// in ms + one decimal position
// now, run some code, and collect some user metrics
	char mytime[100];
	pthread_t id = pthread_self();
	pid_t tidb = syscall(SYS_gettid);
// 	pid_t pidb =getpid();
	add_tid_to_report(my_thread_report->taskid,tidb);//my_thread_report memory reserved on mf_start
	worker_t *me;
	unsigned long workerId = my_thread_report->workerId;
	me = &(workInfo.workers[workerId]);
// 	gmp_printf ("fixed point mpf %.*Ff \n", 4, me->rop);
	chudnovsky( me->rop, workInfo.precision, me->iterStart, me->iterFinish);
	int i=0;
		llint_to_string_alloc(mycurrenttime(),mytime);// <<-- TIME
		printf("\n Processing thread named as: %s\n",my_thread_report->taskid);
		if(pthread_equal(id,workInfo.workers[0].thread)) {
			register_user_metric( my_thread_report, mytime, "n_ships_found", 10*i+5, 0);
		} else
			register_user_metric( my_thread_report, mytime, "number_of_blocks", 20*i+8, 0);
		register_user_metric( my_thread_report, mytime, "counter", i, 1);
		user_metrics_buffer(*my_thread_report);
//end of the thread or component
	printf("\n Finishing thread named as: %s\n",my_thread_report->taskid);
	my_thread_report->end_time=mycurrenttime();
	return NULL;
}

void create_threads(unsigned long threadCount, mpf_t rop, unsigned long precision, unsigned long itercount, struct app_report_t *my_app_report) {
	memset( &workInfo, 0, sizeof(workInfo_t) );
	// Initialization and creation of workers
	workInfo.precision = precision;
	workInfo.workers = malloc(sizeof(worker_t) * threadCount);
	memset(workInfo.workers, 0, sizeof(worker_t) * threadCount);
	for (unsigned long i=0; i<(threadCount-1); i++) {
		mpf_init2(workInfo.workers[i].rop, precision);
		workInfo.workers[i].iterStart = roundDiv(itercount, threadCount) * i;
		workInfo.workers[i].iterFinish = roundDiv(itercount, threadCount) * (i+1);
		my_app_report->my_thread_report[i]->workerId = i;
		pthread_create( &(workInfo.workers[i].thread), NULL, doThread, (void *) (my_app_report->my_thread_report[i]));
	}
	mpf_init2(workInfo.workers[threadCount-1].rop, precision);
	workInfo.workers[threadCount-1].iterStart = roundDiv(itercount, threadCount) * (threadCount-1);
	workInfo.workers[threadCount-1].iterFinish = itercount;
	my_app_report->my_thread_report[threadCount-1]->workerId = 1;
	pthread_create( &(workInfo.workers[threadCount-1].thread), NULL, doThread, (void *) (my_app_report->my_thread_report[threadCount-1]));
	// esperar que cada worker acabe
	for (unsigned long i=0; i<threadCount; i++) {
		pthread_join( workInfo.workers[i].thread, NULL);
		mpf_add(rop, rop, workInfo.workers[i].rop);
	}
	mpf_ui_div(rop, 1, rop);
	for (unsigned long i=0; i<(threadCount); i++)
		mpf_clear(workInfo.workers[i].rop);
	free(workInfo.workers);
}

void my_start_monitoring( const char *mf_server, const char *exec_manager, const char *exec_id, const char* resource_manager, const char *platform_id, struct app_report_t *my_app_report, const char *token){
/* MONITORING METRICS: optional definition, this will merge over the default one loaded from the Resource Manager */
	metrics m_resources;
	m_resources.num_metrics = 3;
	if (m_resources.num_metrics>MAX_NUM_METRICS){
		printf("ERROR: the maximum amount of plugin metrics is %i\n",MAX_NUM_METRICS);
		exit(1);
	}
	m_resources.local_data_storage = 1; /*remove the file if user unset keep_local_data_flag */
	m_resources.sampling_interval[0] = 10; // 1000 stands for 1s
	strcpy(m_resources.metrics_names[0], "resources_usage");//METRIC_NAME_1
	m_resources.sampling_interval[1] = 10; // ms
	strcpy(m_resources.metrics_names[1], "power");//METRIC_NAME_3
	m_resources.sampling_interval[2] = 10; // ms
	strcpy(m_resources.metrics_names[2], "xlnx_monitor");//METRIC_NAME_2
// 	m_resources.sampling_interval[3] = 10; // ms
// 	strcpy(m_resources.metrics_names[3], "nvml");//METRIC_NAME_4
/* MONITORING START */
	mf_start(mf_server, exec_manager, exec_id, resource_manager, platform_id, &m_resources,my_app_report,token);//returns char *datapath
}

int main(int argc, char* argv[]) {
	if (argc<5) {
		fprintf(stderr, "Missing arguments..\n");
		fprintf(stderr, "\t%s exec_id token Nr_iterations  Precision_in_bytes\n", argv[0]);
		return FAILURE;
	}
	char exec_id[200]={'\0'};//currentid: common UNIQUE id for register in the MF all the threads-metrics in this execution.
	strcpy(exec_id, argv[1]);
	char token[256]={'\0'};
	strcpy(token, argv[2]);
	printf("exec_id is :%s\n",exec_id);
	unsigned long itercount = atoi(argv[3]);
	unsigned long precision = 8*atoi(argv[4]);
	if (itercount < total_num_threads) {
		fprintf(stderr, "Error, We can not have more threads than iterations\n");
		return FAILURE;
	}	
	/* init arguments */
	char mf_server[]="localhost:3033";// 141.58.0.8
	char exec_manager[]="localhost:8700";// 141.58.0.8
	char resource_manager[]="localhost:8600";// 141.58.0.8
	char appid[]="montanana_demo";
	char regplatformid[]="node01";
	char execfile[]="pthread-example";
	/******************** Initialize of some registers ************************/
	struct app_report_t *my_app_report= reserve_app_report(total_num_threads, exec_id);//argv[1] is a text string for later friendly query the results
	my_app_report->my_thread_report[0]->total_metrics=0;
	my_app_report->my_thread_report[1]->total_metrics=0;
	strcpy(my_app_report->my_thread_report[0]->taskid,"component_a");
	strcpy(my_app_report->my_thread_report[1]->taskid,"component_b");
	/********************MONITORNG START ************************/
	my_start_monitoring(mf_server, exec_manager, exec_id, resource_manager, regplatformid, my_app_report, token);
	my_app_report->start_app = mycurrenttime();
	/********************* THE APPLICATION- WE CREATE SOME THREADS ***********************************/
	printf("\n Starting program\n");
	mpf_t pi;
	mpf_init2(pi, precision);
	create_threads(total_num_threads, pi, precision, itercount, my_app_report);
	char *pi_str;
	gmp_asprintf( &pi_str, "%.Ff", pi);
	unsigned significantDigits = strlen(pi_str) - 2;
	printf("Nr decimal places: %u\n%s\n", significantDigits, pi_str);
	mpf_clear(pi);
	free(pi_str);
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
