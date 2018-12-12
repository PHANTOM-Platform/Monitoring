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
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include "mf_dummy_connector.h"

#define SUCCESS 0
#define FAILURE 1

#define HAS_TEST1_STAT 0x01

/*******************************************************************************
 * Variable Declarations
 ******************************************************************************/
/* flag indicates which events are given as input */
unsigned int flag = 0;
/* time in seconds */
double before_time, after_time;

#define RESOURCES_EVENTS_NUM 2
const char dummy_supported_metrics[RESOURCES_EVENTS_NUM][32] = { "test1", "test2" };

/*******************************************************************************
 * Functions implementation
 ******************************************************************************/
/* Adds events to the data->events, if the events are valid */
int flag_init(char **events, size_t num_events) {
	int i, ii;
	for (i=0; i < num_events; i++) {
		for (ii = 0; ii < RESOURCES_EVENTS_NUM; ii++) {
			/* if events name matches */
			if(strcmp(events[i], dummy_supported_metrics[ii]) == 0) {
				/* get the flag updated */
				unsigned int current_event_flag = 1 << ii;
				flag = flag | current_event_flag;
			}
		}
	}
	if (flag == 0) {
		fprintf(stderr, "Wrong given metrics.\nPlease given metrics ");
		for (ii = 0; ii < RESOURCES_EVENTS_NUM; ii++)
			fprintf(stderr, "%s ", dummy_supported_metrics[ii]);
		fprintf(stderr, "\n");
		return FAILURE;
	} else 
		return SUCCESS;
}

/** @brief Initializes the dummy plugin
 *
 *  Check if input events are valid; add valid events to the data->events
 *  acquire the previous value and before timestamp
 *
 *  @return 1 on success; 0 otherwise.
 */
int mf_dummy_init(Plugin_metrics *data, char **events, size_t num_events) {
	/* failed to initialize flag means that all events are invalid */
	if(flag_init(events, num_events) == 0)
		return FAILURE;
	
	/* get the before timestamp in second */
	//struct timespec timestamp;
	//clock_gettime(CLOCK_REALTIME, &timestamp);
	//before_time = timestamp.tv_sec * 1.0  + (double)(timestamp.tv_nsec / 1.0e9);
	
	/* initialize Plugin_metrics events' names according to flag */
	int i = 0;
	if(flag & HAS_TEST1_STAT) {
		data->events[i] = malloc(MAX_EVENTS_LEN * sizeof(char));	
		strcpy(data->events[i], "TEST1_usage_rate");
		i++;
	}
	//and next collect other metrics, like TEST2 ...
	data->num_events = i;
	return SUCCESS;
}

/* Gets ram usage rate (unit is %); return the ram usgae rate on success; 0.0 otherwise */
int test1_read() {
	int test1_rate = 0.0;
	test1_rate = rand() % 100;
	return test1_rate;
}

/** @brief Samples all possible events and stores data into the Plugin_metrics
 *
 *  @return 1 on success; 0 otherwise.
 */
int mf_dummy_sample(Plugin_metrics *data) {
	/* get current timestamp in second */
	struct timespec timestamp;
	int i;
	
	for (i = 0; i < data->num_events; i++)
		data->values[i] = 0.0; 
	
	clock_gettime(CLOCK_REALTIME, &timestamp);
	after_time = timestamp.tv_sec * 1.0  + (double)(timestamp.tv_nsec / 1.0e9);
	//double time_interval = after_time - before_time;

	i = 0;
	if(flag & HAS_TEST1_STAT) {
		data->values[i] = (float) test1_read();
		i++;
	}
	//and next collect other metrics, like TEST2 ...
	/* update timestamp */
	//before_time = after_time;
	return SUCCESS;
}

/** @brief Formats the sampling data into a json string
 *
 *  json string contains: plugin name, timestamps, metrics_name and metrics_value
 *
 */
void mf_dummy_to_json(Plugin_metrics *data, char *json) {
    char tmp[128] = {'\0'};
    int i;
    /*
     * prepares the json string, including current timestamp, and name of the plugin
     */
    sprintf(json, "\"type\":\"dummy\"");
    sprintf(tmp, ",\"local_timestamp\":\"%.1f\"", after_time * 1.0e3);
    strcat(json, tmp);

    /*
     * filters the sampled data with respect to metrics values
     */
	for (i = 0; i < data->num_events; i++) {
		/* if metrics' value >= 0.0, append the metrics to the json string */
// 		if(data->values[i] >= 0.0) {
			sprintf(tmp, ",\"%s\":%.3f", data->events[i], data->values[i]);
			strcat(json, tmp);
// 		}
	}
}
