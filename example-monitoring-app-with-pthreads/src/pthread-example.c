#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include "monitoring_support.h"
pthread_t tid[2];

void* doSomeThing( void *args){
	struct Thread_report *my_thread_report;
	my_thread_report = (struct Thread_report *)args;
	my_thread_report->start_time=mycurrenttime();
	pthread_t id = pthread_self();
	char component_name[100];
	if(pthread_equal(id,tid[0])) {
		strcpy(component_name, "first_thread");
		printf("\n First thread processing named as: %s\n",component_name); fflush(stdout);
		my_thread_report->number_of_blocks=117;
	} else {
		strcpy(component_name, "second_thread");
		printf("\n Second thread processing named as: %s\n",component_name); fflush(stdout);
		my_thread_report->n_ships_found=78;
	}  
	my_thread_report->end_time=mycurrenttime();
    return NULL;
}

int main(int argc, char* argv[]) {
	/********************MONITORNG START ************************/
    struct Thread_report all_thread_reports[2];
	char server[]="localhost:3033";
	char regplatformid[]="laptop_montanana";
	char appid[]="demo";
	char execfile[]="pthread-example";
	start_monitoring(server, regplatformid);
	strcpy(all_thread_reports[0].taskid,"main");
	strcpy(all_thread_reports[1].taskid,"update_ship_report");
	//currentid: common UNIQUE id for all the threads in this execution.
	char currentid[100];
	if(argc>1)
		strcpy(currentid,argv[1]);
	else
		strcpy(currentid,"missingid");
	for(int num_thread=0;num_thread<2;num_thread++){
		all_thread_reports[num_thread].n_ships_found=0;
        all_thread_reports[num_thread].number_of_blocks=0;
	}
	/************************************************************/
    for(int i=0;i<2;i++){
        int err = pthread_create(&(tid[i]), NULL, &doSomeThing, (void *) (&all_thread_reports[i]));
        if (err != 0)
            printf("\n Can't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");
    }
    for(int i=0;i<2;i++)
	  (void) pthread_join(tid[i], NULL);
	/************* MONITORING END *******************************/ 
	register_workflow( server, regplatformid, appid, execfile);
	for(int i=0;i<2;i++) 
		prepare_user_metrics(currentid, all_thread_reports[i]);
	stop_monitoring( server, appid, execfile, regplatformid);
	for(int i=0;i<2;i++){
		printf(" execution label of the workflow: \"%s\"\n", currentid);
		printf("   THREAD num %i, name : %s\n",i, all_thread_reports[i].taskid);
		printf("   num of ships : %i\n", all_thread_reports[i].n_ships_found);
		printf("   num of blocks: %u\n", all_thread_reports[i].number_of_blocks);
		printf("   Start time: %llu ns\n", all_thread_reports[i].start_time);
		printf("   End time  : %llu ns\n\n", all_thread_reports[i].end_time);
	}
	/************************************************************/
    return 0;
}
