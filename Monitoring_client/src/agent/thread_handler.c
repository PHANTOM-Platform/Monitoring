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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "main.h" 			// variables like confFile
#include "mf_parser.h" 		// functions like mfp_parse(), ...
#include "publisher.h" 		// function like publish_json()
#include "mf_debug.h"  		// functions like log_error(), log_info()...
#include "plugin_manager.h"	// functions like PluginManager_new(), PluginManager_free(), PluginManager_get_hook()
#include "plugin_discover.h" // variables like pluginCount, plugins_name;
							// functions like discover_plugins(), cleanup_plugins()
#include "thread_handler.h"

#define JSON_LEN 1024
#define SUCCESS 0
#define FAILURE 1

/*******************************************************************************
* Variable Declarations
******************************************************************************/
int running;
int bulk_size;
static PluginManager *pm;
pthread_t threads[256];
long timings[256];
struct timespec sleep_tims[256];
PluginHook *hooks;

/*******************************************************************************
* Forward Declarations
******************************************************************************/
static void catcher(int signo);
static void init_timings(void);
static void *entryThreads(void *arg );  //threads entry for all threads
static int checkConf(void);
static int gatherMetric(struct thread_args *params);

/*******************************************************************************
* Functions implementation
******************************************************************************/
/* All threads starts here */
int startThreads(const char *metrics_publish_URL, const char *token, const char *device_id) {
	int t;
	running = 1;
	/* initialize plugin manager, which is a queue of plugin hooks */
	pm = PluginManager_new();
// 	printf("pointer 1=%p  2=%p\n", (void *) pm, (void *) pm->hook_queue);
	const char *dirname = { "/plugins" };
	char *pluginLocation = malloc(256 * sizeof(char));
	strcpy(pluginLocation, "../dist/bin/");//at the Monitoring_client folder !!! 
	strcat(pluginLocation, dirname);

	/* discover plugins and register them to the plugin manager */ 
	void* pdstate = discover_plugins(pluginLocation, pm);
	if(pdstate==NULL){
		printf("discover_plugins == NULL !!!\n");
		exit(1);
	}
	/* get sampling interval for each plugin */
	init_timings();

	/*get bulk_size from mf_config.ini */
	char tmp_string[20] = {'\0'};
	mfp_get_value("generic", "bulk_size", tmp_string);
	bulk_size = atoi(tmp_string);

	int num_threads = pluginCount ;// why + 1;
	int iret[num_threads];
// 	int nums[num_threads];

	hooks = (PluginHook *)malloc(pluginCount * sizeof(PluginHook));
// 	printf("pointer 3=%p  \n", (void *) hooks);
	for (t = 0; t < pluginCount; t++){
		hooks[t] = PluginManager_get_hook(pm);
		if(hooks[t]==NULL){
			printf(" hooks[t] == NULL !!!\n");
			exit(1);
		}
	}

	struct thread_args params[num_threads];

// 	printf(" done malloc hooks %i\n",pluginCount);
// 	printf(" num_threads is %i\n",num_threads);
	/* create threads for monitoring and updating configurations */
	for (t = 0; t < num_threads; t++) {
		printf("starting thread metrics_publish_URL %s token %s device_id %s ..........\n", metrics_publish_URL, token, device_id);
		params[t].num = t;
		strcpy(params[t].token, token);
		strcpy(params[t].metrics_publish_URL, metrics_publish_URL);
		strcpy(params[t].device_id, device_id);
		
		iret[t] = pthread_create(&threads[t], NULL, entryThreads, &params[t]);
		if (iret[t]) {
			log_error("pthread_create() failed for %s.\n", strerror(iret[t]));
			exit(FAILURE);
		}
	}
	struct sigaction sig;
	sig.sa_handler = catcher; /* signal handler is "catcher" */
	sig.sa_flags = SA_RESTART;
	sigemptyset(&sig.sa_mask);
	sigaction(SIGTERM, &sig, NULL);
	sigaction(SIGINT, &sig, NULL);
	while (running)
		sleep(1);
	/* thread join from plugins threads till all the sending threads */
	for (t = 0; t < pluginCount; t++)
		pthread_join(threads[t], NULL);
	cleanup_plugins(pdstate);
	PluginManager_free(pm);
	free(pluginLocation);
	return SUCCESS;
}

/* catch the stop signal */
static void catcher(int signo) {
	running = 0;
	log_info("Signal %d catched.\n", signo);
}

/* entry for all threads */
static void* entryThreads(void *arg ) {
	struct thread_args *typeT= (struct thread_args*) arg;
	int num = typeT->num;
	if(num < pluginCount){
		printf(" num %i count %i\n",num,pluginCount); fflush(stdout);
		gatherMetric(arg );
	}else
		checkConf();
	return NULL;
}

/* timings update if mf_config.ini has been modified */
static int checkConf(void) {
	while (running) {
		mfp_parse(confFile);
		char wait_some_seconds[20] = {'\0'};
		mfp_get_value("timings", "update_configuration", wait_some_seconds);
		sleep(atoi(wait_some_seconds));
		/* update sleep_tims for all plugins */
		init_timings();
	}
	return SUCCESS;
}


/* each plugin gathers its metrics at a specific rate and send the json-formatted metrics to mf_server */
static int gatherMetric(struct thread_args *params) {
// 	int i;
	long timings = sleep_tims[params->num].tv_sec * 10e8 + sleep_tims[params->num].tv_nsec;
	log_info("Gather metrics of plugin %s (#%d) with update interval of %ld ns\n", plugins_name[params->num], params->num, timings);
	printf("Gather metrics of plugin %s (#%d) with update interval of %ld ns\n", plugins_name[params->num], params->num, timings);
	
// 	char *json_array = calloc(JSON_LEN * bulk_size, sizeof(char));
// 	json_array[0] = '[';
	char msg[JSON_LEN] = {'\0'};
// 	char static_json[512] = {'\0'};

// 	if(experiment_id == NULL){
// 		printf("ERROR experiment_id == NULL\n");
// 		log_error("%s","ERROR experiment_id == NULL.\n");
// 		running= 0;
// 	}else if(experiment_id[0] == '\0'){
// 		printf("ERROR experiment_id == ''\n");
// 		log_error("%s","ERROR experiment_id == ''.\n");
// 		running= 0;
// 	}
// 	sprintf(static_json, "{\"WorkflowID\":\"%s\",\"ExperimentID\":\"%s\",\"TaskID\":\"%s\",\"host\":\"%s\",", 
// 		application_id, experiment_id, task_id, platform_id);
	char *new_json =  malloc(512 * sizeof(char));
	int supported=1;
	while (running) {
// 		for(i=0; i<bulk_size; i++) {
			memset(msg, '\0', JSON_LEN * sizeof(char));
			if(plugins_name[params->num]==NULL)
				exit (0);
			char *json=NULL;
			if( strcmp(plugins_name[params->num],"mf_plugin_Linux_sys_power")== 0){
				if(supported==1){
					json = hooks[params->num]( &supported);	//malloc of json in hooks[params->num]()
// 				}else{ printf("suppressed call of not available metrics");
				}
			}else{
				if(hooks[params->num]==NULL){
					printf(" hooks[params->num] == NULL !!!\n");
					exit(1);
				}
				json = hooks[params->num](params->device_id);	//malloc of json in hooks[params->num]()
			}
			
			const char operation[]="POST";
			struct url_data response;
			response.size=0;
			response.data=NULL;
			response.headercode=NULL;
			strcpy(new_json, "{");
			strcat(new_json, json);
			strcat(new_json, "}");

			printf(" URL: %s\n", params->metrics_publish_URL);
			printf(" operation: %s\n", operation);
			printf(" token: %s\n", params->token);
			printf(" json: %s\n", new_json);
			
			query_message_json(params->metrics_publish_URL, new_json , NULL, &response, operation, params->token);
			
			if(response.headercode!=NULL) free(response.headercode);
			response.headercode=NULL;
			if(response.data == NULL) {
				printf(" response is %s\n",response.data);
				printf("ERROR: Cannot register status on server %s (N)\n", params->metrics_publish_URL);
// 				return FAILURE;
			}else if(response.data[0] == '\0') {
				printf(" response is %s\n",response.data);
				printf("ERROR: Cannot register status on  server %s\n", params->metrics_publish_URL);
// 				return FAILURE;
			}
			if(json != NULL) {
// 				sprintf(msg, "%s%s},", static_json, json);
// 				strcat(json_array, msg);
				free(json);
			}
// 			if(nanosleep(&sleep_tims[params->num], NULL) != 0) {
				/*nano sleep failed,  in case that
				the call is interrupted by a signal handler or encounters an error */
// 				break;
// 			};
// 		}
// 		json_array[strlen(json_array) -1] = ']';
// 		json_array[strlen(json_array)] = '\0';
// 		debug("JSON sent is :\n%s\n", json_array);
		
// 		publish_json(params->metrics_publish_URL, json_array, params->token);
// 		memset(json_array, '\0', JSON_LEN * bulk_size * sizeof(char));
// 		json_array[0] = '[';
		sleep(1);
	}
	free(new_json);
// 	if(json_array != NULL)
// 		free(json_array);
	return SUCCESS;
}

/* parse mf_config.ini to get all timing information */
static void init_timings(void) {
	char *ptr;
	char timing[20] = {'\0'};
	mfp_get_value("timings", "default", timing);
	long default_timing = strtol(timing, &ptr, 10);
	for (int i = 0; i < pluginCount; i++) {
		if (plugins_name[i] == NULL)
			continue;
		char value[20] = {'\0'};
		char *ptr;
		mfp_get_value("timings", plugins_name[i], value);
// printf(" renaming ... plugins_name %s\n",plugins_name[i]);
		if (value[0] == '\0') {
			timings[i] = default_timing;
		} else {
			timings[i] = strtol(value, &ptr, 10);
			log_info("Timing for plugin %s is %ldns\n", plugins_name[i], timings[i]);
			/* update the sleep_tims for the plugin */
			if (timings[i] >= 10e8) {
				sleep_tims[i].tv_sec = timings[i] / 10e8;
				sleep_tims[i].tv_nsec = timings[i] % (long) 10e8;
			} else {
				sleep_tims[i].tv_sec = 0;
				sleep_tims[i].tv_nsec = timings[i];
			}
		}
	}
}
