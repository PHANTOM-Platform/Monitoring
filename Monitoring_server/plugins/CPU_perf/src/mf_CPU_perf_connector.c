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
#include <papi.h>
#include "mf_CPU_perf_connector.h"

#define SUCCESS 1
#define FAILURE 0
#define PAPI_EVENTS_NUM 3

/*******************************************************************************
 * Variable Declarations
 ******************************************************************************/
static int DEFAULT_CPU_COMPONENT = 0;
const int PAPI_EVENTS[PAPI_EVENTS_NUM] = {PAPI_FP_INS, PAPI_FP_OPS, PAPI_TOT_INS};
const char CPU_perf_metrics[PAPI_EVENTS_NUM][16] = {"MFLIPS", "MFLOPS", "MIPS"};
int *EventSet = NULL;
long long before_time, after_time;

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
int events_are_all_not_valid(char **events, size_t num_events);
static int load_papi_library(int *num_cores);

/*******************************************************************************
 * Functions implementation
 ******************************************************************************/
/** @brief Initializes the CPU_perf plugin
 *
 *  Load papi library; create a EventSet; add available events to the EventSet;
 *  get the start timestamp; and start the counters specified in the generated EventSet.
 *
 *  @return 1 on success; 0 otherwise.
 */
int mf_CPU_perf_init(Plugin_metrics *data, char **events, size_t num_events, int num_cores)
{
	int i, ii, jj;
	data->num_events = 0;//in case of failure the value must be cero
	/* if all given events are not valid, return directly */
	if (events_are_all_not_valid(events, num_events)) {
		return FAILURE;
	}

	/* load papi library and set the number of cores no bigger than the maximum number of cores available */
	if (!load_papi_library(&num_cores)) {
        return FAILURE;
    }

    /* create eventset and set options for each cpu core */
    EventSet = malloc(num_cores * sizeof(int));
    for(i = 0; i < num_cores; i++) {
    	EventSet[i] = PAPI_NULL;
    	if (PAPI_create_eventset(&EventSet[i]) != PAPI_OK) {
			fprintf(stderr, "PAPI_create_eventset for core %d failed.\n", i);
			return FAILURE;
		}
		
    	if (PAPI_assign_eventset_component(EventSet[i], DEFAULT_CPU_COMPONENT) != PAPI_OK) {
        	fprintf(stderr, "PAPI_assign_eventset_component for core %d failed.\n", i);
			return FAILURE;
    	}

    	PAPI_domain_option_t domain_opt;
    	domain_opt.def_cidx = DEFAULT_CPU_COMPONENT;
    	domain_opt.eventset = EventSet[i];
    	domain_opt.domain = PAPI_DOM_ALL;
    	if (PAPI_set_opt(PAPI_DOMAIN, (PAPI_option_t*) &domain_opt) != PAPI_OK) {
        	fprintf(stderr, "PAPI_set_opt for core %d for PAPI_DOMAIN failed.\n", i);
			return FAILURE;
    	}

    	PAPI_granularity_option_t gran_opt;
    	gran_opt.eventset = EventSet[i];
	    gran_opt.granularity = PAPI_GRN_SYS;
	    if (PAPI_set_opt(PAPI_GRANUL, (PAPI_option_t*) &gran_opt) != PAPI_OK) {
    	    fprintf(stderr, "PAPI_set_opt for core %d for PAPI_GRANUL failed.\n", i);
			return FAILURE;
    	}

    	PAPI_cpu_option_t cpu_opt;
    	cpu_opt.eventset = EventSet[i];
	    cpu_opt.cpu_num = i;
	    if (PAPI_set_opt(PAPI_CPU_ATTACH, (PAPI_option_t*) &cpu_opt) != PAPI_OK) {
    		fprintf(stderr, "PAPI_set_opt for core %d for PAPI_CPU_ATTACH failed.\n", i);
			return FAILURE;
    	}

    }

	/* check if the papi events available, creat an EventSet for all available events */
	for(i = 0, jj = 0; i < num_cores; i++) {
		for (ii = 0; ii < PAPI_EVENTS_NUM; ii++) {
			if ( PAPI_query_event(PAPI_EVENTS[ii]) == PAPI_OK ) {
				PAPI_add_event(EventSet[i], PAPI_EVENTS[ii]);
				data->events[jj] = malloc(MAX_EVENTS_LEN * sizeof(char));
				sprintf(data->events[jj], "core%02d:%s", i, CPU_perf_metrics[ii]);
				jj++;
				if(jj== MAX_EVENTS_NUMBER){
					printf("error, overflow on number of events\n");
					exit(1);
				}
			}
		}
	}
	data->num_events = jj;

	/* Start counting events */
	before_time = PAPI_get_real_nsec();
	
	for(i = 0; i < num_cores; i++) {
		if (PAPI_start(EventSet[i]) != PAPI_OK) {
			fprintf(stderr, "PAPI_start failed.\n");
			return FAILURE;
		}
	}	
	return SUCCESS;
}

/** @brief Samples all possible events and stores data into the Plugin_metrics
 *
 *  @return 1 on success; 0 otherwise.
 */
int mf_CPU_perf_sample(Plugin_metrics *data, int num_cores)
{
	int ret, i, ii, jj;
	long long values[PAPI_EVENTS_NUM];
	long long duration;

	for(i = 0; i < PAPI_EVENTS_NUM; i++) {
		values[i]=0;//need inintialize in case the PAPI_read doen't collect data
	}
	
	after_time = PAPI_get_real_nsec();
	duration = after_time - before_time; /* in nanoseconds */
	
	jj = 0;
	for(i = 0; i < num_cores; i++) {
		ret = PAPI_read(EventSet[i], values);
		if(ret != PAPI_OK) {
			char *error = PAPI_strerror(ret);
			fprintf(stderr, "Error while reading the PAPI counters: %s", error);
        	return FAILURE;
		}
		for(ii = 0; ii < PAPI_EVENTS_NUM; ii++) {
			data->values[jj] = (float) (values[ii] * 1.0e3) / duration; /*units are Mflips, Mflops, and Mips */
			jj++;
			if(jj== MAX_EVENTS_NUMBER){
				printf("error, overflow on number of events\n");
				exit(1);
			}
		}
		PAPI_reset(EventSet[i]);	
	}
	//jj should be equal to data->num_events 
	before_time = after_time;
	
	return SUCCESS;
}

/** @brief Formats the sampling data into a json string
 *
 *  json string contains: plugin name, timestamps, metrics_name and metrics_value
 *
 */
void mf_CPU_perf_to_json(Plugin_metrics *data, char **events, size_t num_events, char *json)
{
	struct timespec timestamp;
    char tmp[128] = {'\0'};
    int i, ii;
    /*
     * prepares the json string, including current timestamp, and name of the plugin
     */
    sprintf(json, "\"type\":\"CPU_perf\"");
    clock_gettime(CLOCK_REALTIME, &timestamp);
    double ts = timestamp.tv_sec * 1.0e3 + (double)(timestamp.tv_nsec / 1.0e6); // in millisecond
    sprintf(tmp, ",\"local_timestamp\":\"%.1f\"", ts);
    strcat(json, tmp);

    /*
     * filters the sampled data with respect to metrics given
     */
	for (i = 0; i < num_events; i++) {
		for(ii = 0; ii < data->num_events; ii++) {
			/* if metrics' name matches, append the metrics to the json string */
			if((strstr(data->events[ii], events[i]) != NULL)  ) {
				if( (data->values[ii] > 0.0)) { 
					sprintf(tmp, ",\"%s\":%.3f", data->events[ii], data->values[ii]);
					strcat(json, tmp);
				}
			}
		}
	}
}

/** @brief Stops the plugin
 *
 *  This methods stops papi counters gracefully;
 *
 */
void mf_CPU_perf_shutdown(int num_cores)
{
	int ret, i;
	for (i=0; i < num_cores; i++) {
		ret = PAPI_stop(EventSet[i], NULL);
	    if (ret != PAPI_OK) {
    	    char *error = PAPI_strerror(ret);
        	fprintf(stderr, "Couldn't stop PAPI EventSet: %s", error);
	    }

    	ret = PAPI_cleanup_eventset(EventSet[i]);
    	if (ret != PAPI_OK) {
        	char *error = PAPI_strerror(ret);
	        fprintf(stderr, "Couldn't cleanup PAPI EventSet: %s", error);
    	}

	    ret = PAPI_destroy_eventset(&EventSet[i]);
    	if (ret != PAPI_OK) {
	        char *error = PAPI_strerror(ret);
    	    fprintf(stderr, "Couldn't destroy PAPI EventSet: %s", error);
    	}
	}
	PAPI_shutdown();
}

/* Load the PAPI library */
static int load_papi_library(int *num_cores)
{
    if (PAPI_is_initialized()) {
        return SUCCESS;
    }

    int ret = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret != PAPI_VER_CURRENT) {
        char *error = PAPI_strerror(ret);
        fprintf(stderr, "Error while loading the PAPI library: %s", error);
        return FAILURE;
    }

    int max_cores = PAPI_get_opt(PAPI_MAX_CPUS,/*@-nullpass@*/ NULL);
    if (max_cores <= 0) {
        return FAILURE;
    }
    /* set the number of cores not bigger than the maximum number of cores available */
    if (*num_cores > max_cores) {
        *num_cores = max_cores;
    }

    return SUCCESS;
}

/* Checks if all events are not valid; return 1 when all events are not valid; 0 otherwise. */
int events_are_all_not_valid(char **events, size_t num_events) 
{
	int i, ii, counter;
	counter = 0; 
	for (i=0; i < num_events; i++) {
		for (ii = 0; ii < PAPI_EVENTS_NUM; ii++) {
			/* if events name matches, counter is incremented by 1 */
			if(strcmp(events[i], CPU_perf_metrics[ii]) == 0) {
				counter++;
			}
		}
	}
	if (counter == 0) {
		fprintf(stderr, "Wrong given metrics.\nPlease given metrics MFLIPS, MFLOPS, or MIPS\n");
		return 1;
	} else {
		return 0;
	}
}
