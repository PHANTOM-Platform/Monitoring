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
#include <hwloc.h>
#include <papi.h>
#include "mf_RAPL_power_connector.h"

#define SUCCESS 0
#define FAILURE 1
/*******************************************************************************
* Variable Declarations
******************************************************************************/
double before_time, after_time; /* time in seconds */
int EventSet = PAPI_NULL;
int num_sockets = 0;
double denominator = 1.0 ; /*according to different CPU models, DRAM energy scalings are different*/
int rapl_is_available = 0;
#define max_supported_sockets 4
float epackage_before[max_supported_sockets],
	edram_before[max_supported_sockets],
	epackage_after[max_supported_sockets],
	edram_after[max_supported_sockets];

/*******************************************************************************
* Forward Declarations
******************************************************************************/
int rapl_init(Plugin_metrics *data, char **events, size_t num_events);
int load_papi_library(void);
int check_rapl_component(void);
int hardware_sockets_count(void);
double rapl_get_denominator(void);
void native_cpuid(unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
int rapl_stat_read(float *epackage, float *edram);
/*******************************************************************************
* Functions implementation
******************************************************************************/
/** @brief Initializes the papi library; check if rapl component is enabled; read the initial power measurements and timestamp
*
* @return 1 on success; 0 otherwise.
*/
int mf_RAPL_power_init(Plugin_metrics *data, char **events, size_t num_events) {
	rapl_is_available = rapl_init(data, events, num_events);
	if(rapl_is_available == 0 || data->num_events == 0)
		return FAILURE;
	rapl_stat_read(epackage_before, edram_before);
	/* get the before timestamp in second */
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
	before_time = timestamp.tv_sec * 1.0 + (double)(timestamp.tv_nsec / 1.0e9);
	return SUCCESS;
}

/** @brief Samples and calculates the average power during the sampling interval
* @return 1 on success; 0 otherwise.
*/
int mf_RAPL_power_sample(Plugin_metrics *data) {
	int i, j;
	/* get current timestamp in second */
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
	after_time = timestamp.tv_sec * 1.0 + (double)(timestamp.tv_nsec / 1.0e9);
	double time_interval = after_time - before_time; /* get time interval */
	if(rapl_is_available)
		rapl_stat_read(epackage_after, edram_after);
	for (i = 0; i < data->num_events; ) {
		if((data->events[i] != NULL) && (strstr(data->events[i], "total_power") != NULL)) {
			for (j = 0; j < num_sockets; j++) {
				data->values[i] = (epackage_after[j] - epackage_before[j]) / time_interval;	//unit is milliWatt
				i++;
				epackage_before[j] = epackage_after[j];
			}
		}
		if((data->events[i] != NULL) &&(strstr(data->events[i], "dram_power") != NULL)) {
			for (j = 0; j < num_sockets; j++) {
				data->values[i] = (edram_after[j] - edram_before[j]) / time_interval; //unit is milliWatt
				i++;
				edram_before[j] = edram_after[j];
			}
		}
	}
	/* update timestamp */
	before_time = after_time;
	return SUCCESS;
}

/** @brief Formats the sampling data into a json string
*
* json string contains: plugin name, timestamps, metrics_name and metrics_value
*/
void mf_RAPL_power_to_json(Plugin_metrics *data, char *json) {
	char tmp[128] = {'\0'};
	int i;
	/* prepares the json string, including current timestamp, and name of the plugin */
	sprintf(json, "\"type\":\"RAPL_power\"");
	sprintf(tmp, ",\"local_timestamp\":\"%.1f\"", after_time * 1.0e3);
	strcat(json, tmp);
	/* filters the sampled data with respect to metrics values */
	for (i = 0; i < data->num_events; i++) {
		/* if metrics' value >= 0.0, append the metrics to the json string */
		if(data->values[i] >= 0.0) {
			sprintf(tmp, ",\"%s\":%.3f", data->events[i], data->values[i]);
			strcat(json, tmp);
		}
	}
}

/** initialize RAPL counters prepare eventset and start counters */
int rapl_init(Plugin_metrics *data, char **events, size_t num_events) {
	int i, j, ii = 0;
	char event_name[32] = {'\0'};
	/* Load PAPI library */
	if (load_papi_library()!=SUCCESS)
		return FAILURE;
	/* check if rapl component is enabled */
	if (check_rapl_component()!=SUCCESS)
		return FAILURE;
	/* get the number of sockets */
	num_sockets = hardware_sockets_count();
	if(num_sockets <= 0)
		return FAILURE;
	/* creat an PAPI EventSet */
	if (PAPI_create_eventset(&EventSet) != PAPI_OK) {
		fprintf(stderr, "Error: PAPI_create_eventset failed at rapl_init.\n");
		return FAILURE;
	}
	/* add for each socket the package energy and dram energy events */
	for (i = 0; i < num_sockets; i++) {
		memset(event_name, '\0', 32 * sizeof(char));
		sprintf(event_name, "PACKAGE_ENERGY:PACKAGE%d", i);
		PAPI_add_named_event(EventSet, event_name);

		memset(event_name, '\0', 32 * sizeof(char));
		sprintf(event_name, "DRAM_ENERGY:PACKAGE%d", i);
		PAPI_add_named_event(EventSet, event_name);		
	}
	/* create data events according to given metrics */
	for (i = 0; i < num_events; i++) {
		if(strcmp(events[i], "total_power") == 0) {
			for (j = 0; j < num_sockets; j++) {
				data->events[ii] = malloc(MAX_EVENTS_LEN * sizeof(char));
				sprintf(data->events[ii], "package%d:total_power", j);
				ii++;
			}
		}
		if(strcmp(events[i], "dram_power") == 0) {
			for (j = 0; j < num_sockets; j++) {
				data->events[ii] = malloc(MAX_EVENTS_LEN * sizeof(char));
				sprintf(data->events[ii], "package%d:dram_power", j);
				ii++;
			}
		}
	}
	data->num_events = ii;
	/* set dominator for DRAM energy values based on different CPU model */
	denominator = rapl_get_denominator();
	if (PAPI_start(EventSet) != PAPI_OK) {
		fprintf(stderr, "PAPI_start failed at rapl_init.\n");
		return FAILURE;
	}
	return SUCCESS;
}

/** Load the PAPI library */
int load_papi_library(void) {
	if (PAPI_is_initialized()!=PAPI_NOT_INITED)
		return SUCCESS;
	int ret = PAPI_library_init(PAPI_VER_CURRENT);
	if (ret != PAPI_VER_CURRENT) {
		char *error = PAPI_strerror(ret);
		fprintf(stderr, "Error while loading the PAPI library: %s\n", error);
		return FAILURE;
	}
	return SUCCESS;
}

/** Check if rapl component is enabled */
int check_rapl_component(void) {
	int numcmp, cid; /* number of component and component id variables declare */
	const PAPI_component_info_t *cmpinfo = NULL;
	numcmp = PAPI_num_components();
	for (cid = 0; cid < numcmp; cid++) {
		cmpinfo = PAPI_get_component_info(cid);
		if (strstr(cmpinfo->name, "rapl")) {
			if (cmpinfo->disabled) {
				fprintf(stderr, "Component RAPL is DISABLED\n");
				return FAILURE;
			} else {
				return SUCCESS;
			}
		}
	}
	return FAILURE;
}

/** Count the number of available sockets by hwloc library; return the number of sockets on success; 0 otherwise*/
int hardware_sockets_count(void) {
	int depth;
	int skts_num = 0;
	hwloc_topology_t topology;
	hwloc_topology_init(&topology);
	hwloc_topology_load(topology);
	depth = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
	if (depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
		fprintf(stderr, "Error: The number of sockets is unknown.\n");
		return 0;
	}
	skts_num = hwloc_get_nbobjs_by_depth(topology, depth);
	if (skts_num >max_supported_sockets){
		printf(" WARNING more than max_supported_sockets\n");
		skts_num=max_supported_sockets;
	}
	return skts_num;
}

/** get the coefficient of current CPU model */
double rapl_get_denominator(void) {
	/* get cpu model */
	unsigned int eax, ebx, ecx, edx;
	eax = 1;
	native_cpuid(&eax, &ebx, &ecx, &edx);
	int cpu_model = (eax >> 4) & 0xF;
	if (cpu_model == 15) {
		return 15.3;
	}
	return 1.0;
}

/** Get native cpuid */
void native_cpuid(unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx) {
	asm volatile("cpuid"
		: "=a" (*eax),
		"=b" (*ebx),
		"=c" (*ecx),
		"=d" (*edx)
		: "0" (*eax), "2" (*ecx)
	);
}

/** Read rapl counters values, computer the energy values for CPU and DRAM (in milliJoule); 
*counters are reset after read */
int rapl_stat_read(float *epackage, float *edram) {
	int i, ii, ret;
	long long *values = malloc(2 * num_sockets * sizeof(long long));
	ret = PAPI_read(EventSet, values);
	if(ret != PAPI_OK) {
		char *error = PAPI_strerror(ret);
		fprintf(stderr, "Error while reading the PAPI counters: %s\n", error);
		return FAILURE;
	}
	for(i = 0, ii = 0; ii < num_sockets; ii++) {
		epackage[ii] = (float) (values[i] * 1.0e-6);
		i++;
		edram[ii] = (float) (values[i] * 1.0e-6 ) / denominator;
		i++;
	}
	PAPI_reset(EventSet);
	return SUCCESS;
}
