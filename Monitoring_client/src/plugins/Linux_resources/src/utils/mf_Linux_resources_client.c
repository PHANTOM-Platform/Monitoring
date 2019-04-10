/*
* Copyright (C) 2018 University of Stuttgart
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
#include <signal.h>
#include <time.h>
#include "mf_Linux_resources_connector.h"
#include "plugin_utils.h"
/*******************************************************************************
* Forward Declarations
******************************************************************************/
static void my_exit_handler();

/* mf_Linux_resources_client main function */
int main(int argc, char** argv) {
	if (argc <= 1) {
		printf("Error: No metrics required for monitoring.");
		exit(0);
	}
	const char *device_id="testing_device_id";
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = my_exit_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	/*default sampling interval: 1 second */
	struct timespec profile_time = { 0, 0 };
	profile_time.tv_sec = 1;
	profile_time.tv_nsec = 0;
	++argv;
	--argc;
	/*initialize the plugin */
	Plugin_metrics *monitoring_data = malloc(sizeof(Plugin_metrics));
	int ret = mf_Linux_resources_init(monitoring_data, argv, argc);
	if(ret == 0) {
		printf("Error: Plugin init function failed.\n");
		exit(0);
	}
	do {
		/*sleep for a given time until next sample */
		nanosleep(&profile_time, NULL);
		/* sampling */
		mf_Linux_resources_sample(monitoring_data);
		/* Prepares a json string, including current timestamp, name of the plugin,
		* and required metrics. */
		char *json = calloc(JSON_MAX_LEN, sizeof(char));
		mf_Linux_resources_to_json(monitoring_data, device_id, json);
		/* Display and free the json string */
		puts(json);
		free(json);
	} while (1);
}

/* Exit handler */
static void my_exit_handler(int s) {
	puts("Bye bye!\n");
	exit(0);
}
