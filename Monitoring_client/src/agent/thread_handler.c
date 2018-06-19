/*
 * Copyright 2014, 2015 High Performance Computing Center, Stuttgart
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
#define SUCCESS 1
#define FAILURE 0

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
static void *entryThreads(void *arg);  //threads entry for all threads
static int checkConf(void);
static int gatherMetric(int num);

/*******************************************************************************
 * Functions implementation
 ******************************************************************************/
/* All threads starts here */
int startThreads(void) {
	int t;
	running = 1;

	/* initialize plugin manager, which is a queue of plugin hooks */
	pm = PluginManager_new();
	const char *dirname = { "/plugins" };
	char *pluginLocation = malloc(256 * sizeof(char));
	strcpy(pluginLocation, pwd);
	strcat(pluginLocation, dirname);

	/* discover plugins and register them to the plugin manager */ 
	void* pdstate = discover_plugins(pluginLocation, pm);

	/* get sampling interval for each plugin */
	init_timings();

	/*get bulk_size from mf_config.ini */
	char tmp_string[20] = {'\0'};
	mfp_get_value("generic", "bulk_size", tmp_string);
	bulk_size = atoi(tmp_string);

	int num_threads = pluginCount + 1;
	int iret[num_threads];
	int nums[num_threads];

	hooks = (PluginHook *)malloc(pluginCount * sizeof(PluginHook));
	for (t = 0; t < pluginCount; t++) {
		hooks[t] = PluginManager_get_hook(pm);
	}

	/* create threads for monitoring and updating configurations */
	for (t = 0; t < num_threads; t++) {
		nums[t] = t;
		iret[t] = pthread_create(&threads[t], NULL, entryThreads, &nums[t]);
		if (iret[t]) {
			log_error("pthread_create() failed for %s.\n", strerror(iret[t]));
			exit(FAILURE);
		}
	}

	struct sigaction sig;
	sig.sa_handler = catcher; /* signal handler is "catcher" */
	sig.sa_flags = SA_RESTART;
	sigemptyset(&sig.sa_mask);
	sigaction(SIGTERM, &sig, NULL );
	sigaction(SIGINT, &sig, NULL );

	while (running)
		sleep(1);
	
	/* thread join from plugins threads till all the sending threads */
	for (t = 0; t < pluginCount; t++) {
		pthread_join(threads[t], NULL);
	}

	cleanup_plugins(pdstate);
	PluginManager_free(pm);
	free(pluginLocation);
	return SUCCESS;
}

/* catch the stop signal */
static void catcher(int signo) 
{
	running = 0;
	log_info("Signal %d catched.\n", signo);
}

/* entry for all threads */
static void* entryThreads(void *arg) 
{
	int *typeT = (int*) arg;
	if(*typeT < pluginCount) {
		gatherMetric(*typeT);
	}
	else {
		checkConf();
	}
	return NULL;
}

/* timings update if mf_config.ini has been modified */
static int checkConf(void) 
{
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
static int gatherMetric(int num) 
{
	int i;
	long timings = sleep_tims[num].tv_sec * 10e8 + sleep_tims[num].tv_nsec;

	log_info("Gather metrics of plugin %s (#%d) with update interval of %ld ns\n", plugins_name[num], num, timings);

	char *json_array = calloc(JSON_LEN * bulk_size, sizeof(char));
	json_array[0] = '[';
	char msg[JSON_LEN] = {'\0'};
	char static_json[512] = {'\0'};

	sprintf(static_json, "{\"WorkflowID\":\"%s\",\"ExperimentID\":\"%s\",\"TaskID\":\"%s\",\"host\":\"%s\",", 
		application_id, experiment_id, task_id, platform_id);

	while (running) {

		for(i=0; i<bulk_size; i++) {
			memset(msg, '\0', JSON_LEN * sizeof(char));
			char *json = hooks[num]();	//malloc of json in hooks[num]()
			if(json != NULL) {
				sprintf(msg, "%s%s},", static_json, json);
				strcat(json_array, msg);
				free(json);
			}
			if(nanosleep(&sleep_tims[num], NULL) != 0) {
				/*nano sleep failed,  in case that
				  the call is interrupted by a signal handler or encounters an error */
				break;
			};
		}
		json_array[strlen(json_array) -1] = ']';
		json_array[strlen(json_array)] = '\0';
		debug("JSON sent is :\n%s\n", json_array);
		publish_json(metrics_publish_URL, json_array);
		memset(json_array, '\0', JSON_LEN * bulk_size * sizeof(char));
		json_array[0] = '[';
		
	}

	if(json_array != NULL) {
		free(json_array);
	}
	return SUCCESS;
}

/* parse mf_cconfig.ini to get all timing information */
static void init_timings(void)
{
	char *ptr;
	char timing[20] = {'\0'};
	mfp_get_value("timings", "default", timing);
	long default_timing = strtol(timing, &ptr, 10);

	for (int i = 0; i < pluginCount; i++) {
		if (plugins_name[i] == NULL) {
			continue;
		}
		char value[20] = {'\0'};
		char *ptr;
		mfp_get_value("timings", plugins_name[i], value);
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