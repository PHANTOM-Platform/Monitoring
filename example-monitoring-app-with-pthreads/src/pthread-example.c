#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <stdio.h>
#include <math.h>
#include "mf_api.h"
pthread_t tid[2];

#define PI 3.14159265

//3033 and 2779 are the standard and the hlrs-server ports for the monitoring server.
// const char server[]="141.58.0.8:2779";
const char server[]="localhost:3033";
char currentid[100]; //currentid: common UNIQUE id for register in the MF all the threads-metrics in this execution.
 
void start_monitoring( const char *server, const char *regplatformid){
/* MONITORING METRICS */
	metrics m_resources;
	m_resources.num_metrics = 3;
	m_resources.local_data_storage = 1; /*remove the file if user unset keep_local_data_flag */
	m_resources.sampling_interval[0] = 10; // 1000 stands for 1s, unit in ms
	strcpy(m_resources.metrics_names[0], "resources_usage");
	m_resources.sampling_interval[1] = 10; // unit in ms
	strcpy(m_resources.metrics_names[1], "disk_io");
	m_resources.sampling_interval[2] = 10; // unit in ms
	strcpy(m_resources.metrics_names[2], "power");
/* MONITORING START */
	mf_start(server, regplatformid, &m_resources);
}

void* doSomeThing( void *args){
	unsigned int i,j, angle=0;
	const int n =720;
	struct Thread_report *my_thread_report;
	my_thread_report = (struct Thread_report *)args;
	my_thread_report->start_time=mycurrenttime();
	my_thread_report->total_metrics= 3;
	my_thread_report->user_label  = (char **) malloc(n * sizeof(char*));
	my_thread_report->user_value  = (char **) malloc(n * sizeof(char*)); //it contains a string, need add quotes if the value is text !!!
	my_thread_report->metric_time = (char **) malloc(n * sizeof(char*));
	for (i=0;i<my_thread_report->total_metrics;i++){
		my_thread_report->user_label[i]  = (char *) malloc(40 * sizeof(char));
		my_thread_report->user_value[i]  = (char *) malloc(40 * sizeof(char));
		my_thread_report->metric_time[i] = (char *) malloc(40 * sizeof(char));
	}
	
	pthread_t id = pthread_self(); 
	char component_name[100];
	char temp_i[100];
	char mytime[100];
	for (i=0;i<n;i++){
		llint_to_string_alloc(mycurrenttime(),mytime);// <<-- TIME
		strcat(mytime, ".0");
		angle=(angle+1) % 360;
		for (j=0;j<my_thread_report->total_metrics;j++)
			strcpy(my_thread_report->metric_time[j], mytime);   // <<-- TIME 
		if(pthread_equal(id,tid[0])) {
			strcpy(component_name, "first_thread ... ");
			itoa(i,temp_i);
			strcat(component_name, temp_i);
			printf("\n Processing thread named as: %s\n",component_name);
			strcpy(my_thread_report->user_label[0], "n_ships_found"); // <<-- LABEL
			itoa((int) 10*i+5, my_thread_report->user_value[0]);      // <<-- VALUE
			strcpy(my_thread_report->user_label[2], "simple_function_comp_a"); // <<-- LABEL
			ftoa((float) (40.0+20.0* cosf(((float) angle) * PI / 180.0 ) ) , my_thread_report->user_value[2],3); // <<-- VALUE 
		} else {
			strcpy(component_name, "second_thread ... ");
			itoa(i,temp_i);
			strcat(component_name, temp_i);
			printf("\n Processing thread named as: %s\n",component_name);
			strcpy(my_thread_report->user_label[0], "number_of_blocks"); // <<-- LABEL
			itoa((int) 20*i+8, my_thread_report->user_value[0]);         // <<-- VALUE
			strcpy(my_thread_report->user_label[2], "simple_function_comp_b"); // <<-- LABEL
			ftoa((float) (40.0+20.0* sinf(((float) angle) * PI / 180.0 ) ) , my_thread_report->user_value[2],3); // <<-- VALUE 
		}		
		strcpy(my_thread_report->user_label[1], "counter"); // <<-- LABEL
		itoa((int) i, my_thread_report->user_value[1]);     // <<-- VALUE
		user_metrics_buffer(currentid,*my_thread_report); 
		usleep(10000);  /* sleep unit is on us */
	}
	printf("\n Finishing thread named as: %s\n",component_name);
	fflush(stdout);
	my_thread_report->end_time=mycurrenttime();
    return NULL;
}

int main(int argc, char* argv[]) {
	const int amount_of_threads = 2; //we have 2 threads in this example
	/********************MONITORNG START ************************/
    struct Thread_report all_thread_reports[2];
	char regplatformid[]="node01";
	char appid[]="demo";
	char execfile[]="pthread-example";
	start_monitoring(server, regplatformid);
	strcpy(all_thread_reports[0].taskid,"component_a");
	strcpy(all_thread_reports[1].taskid,"component_b");
	for(int i=0;i<amount_of_threads;i++)
		all_thread_reports[i].total_metrics=0;
	
	if(argc>1)
		strcpy(currentid,argv[1]);
	else
		strcpy(currentid,"missingid");
	/************************************************************/
    for(int i=0;i<amount_of_threads;i++){
        int err = pthread_create(&(tid[i]), NULL, &doSomeThing, (void *) (&all_thread_reports[i]));
        if (err != 0)
            printf("\n Can't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");
    }
    for(int i=0;i<amount_of_threads;i++)
	  (void) pthread_join(tid[i], NULL);
	printf("\n Finishing program\n");
	/************* MONITORING END *******************************/
	register_workflow( server, regplatformid, appid, execfile);
	for(int i=0;i<amount_of_threads;i++)
		register_end_component(currentid, all_thread_reports[i]);
	monitoring_send( server, appid, execfile, regplatformid);
	mf_end();
	for(int i=0;i<amount_of_threads;i++){
		printf(" Execution label of the workflow: \"%s\"\n", currentid);
		printf("   THREAD num %i, name : %s\n",i, all_thread_reports[i].taskid);
		printf("     total metrics %i:\n",all_thread_reports[i].total_metrics);
		for(int j=0;j<all_thread_reports[i].total_metrics;j++)
			printf("   %s : %s\n", all_thread_reports[i].user_label[0], all_thread_reports[i].user_value[0]);
		printf("   Start time: %llu ns\n", all_thread_reports[i].start_time);
		printf("   End time  : %llu ns\n\n", all_thread_reports[i].end_time);
	}
	/************************************************************/
	for (int i=0;i<amount_of_threads;i++){
		for (int j=0;j<all_thread_reports[i].total_metrics;j++ ){
			free(all_thread_reports[i].user_label[j]);
			free(all_thread_reports[i].user_value[j]);
			free(all_thread_reports[i].metric_time[j]);
		}
		free(all_thread_reports[i].user_label); 
		free(all_thread_reports[i].user_value);
		free(all_thread_reports[i].metric_time);
	}
	return 0;
}
