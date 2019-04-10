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
#include <nvml.h>
#include "mf_NVML_connector.h"

#define SUCCESS 0
#define FAILURE 1
#define NVML_EVENTS_NUM 7

/*******************************************************************************
 * Variable Declarations
 ******************************************************************************/
unsigned int devices_count = 0;
nvmlDevice_t **devices = NULL; 
const char NVML_metrics[NVML_EVENTS_NUM][32] = {
	"gpu_usage_rate", "mem_usage_rate", "mem_allocated",
	"PCIe_snd_throughput", "PCIe_rcv_throughput",
	"temperature", "power"
};

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
int events_are_all_not_valid(char **events, size_t num_events);
static int load_nvml_library();

/*******************************************************************************
 * Functions implementation
 ******************************************************************************/
/** @brief Initializes the NVML plugin
 *
 *  Load NVML library; 
 *
 *  @return 1 on success; 0 otherwise.
 */
int mf_NVML_init(Plugin_metrics *data, char **events, size_t num_events)
{
	int i, j, k = 0;
	/* if all given events are not valid, return directly */
	if (events_are_all_not_valid(events, num_events))
		return FAILURE;

	if (!load_nvml_library())
		return FAILURE;

	/* count the number of GPU devices */
	nvmlReturn_t ret = nvmlDeviceGetCount(&devices_count);
	if(ret != NVML_SUCCESS) {
		fprintf(stderr, "mf_NVML_init failed during counting GPU devices.\n");
		return FAILURE;
	}

	/* initialize Plugin_metrics events' names */
	for(i = 0; i < devices_count; i++) {
		for (j = 0; j < NVML_EVENTS_NUM; j++) {
			data->events[k] = malloc(MAX_EVENTS_LEN * sizeof(char));	
			sprintf(data->events[k], "GPU%d:%s", i, NVML_metrics[j]);
			k++;
		}
	}
	data->num_events = k;

	/* get device handle for each GPU device */
	devices = calloc(devices_count, sizeof(nvmlDevice_t *));
	for(i = 0; i < devices_count; i++) {
		devices[i] = malloc(sizeof(nvmlDevice_t));
		ret = nvmlDeviceGetHandleByIndex(i, devices[i]);
		if(ret != NVML_SUCCESS) {
			fprintf(stderr, "mf_NVML_init failed during getting GPU device handle.\n");
			return FAILURE;
		}
	}

	return SUCCESS;
}

/** @brief Samples all possible events and stores data into the Plugin_metrics
 *
 *  @return 1 on success; 0 otherwise.
 */
int mf_NVML_sample(Plugin_metrics *data)
{
	int i, j = 0;
	nvmlReturn_t ret;
	for (i = 0; i < devices_count; i++) {
		nvmlUtilization_t utilization;
		ret = nvmlDeviceGetUtilizationRates(*devices[i], &utilization);
		if(ret == NVML_SUCCESS) {
			/* the value is percent of time over the past second,
			 during which one or more kernels was executing on the GPU */
			data->values[j] = (float) utilization.gpu * 1.0;
			/* the value is percent of time over the past second,
			 during which global (device) memory was being read or written */
			data->values[j+1] = (float) utilization.memory * 1.0;
		} else {
			data->values[j] = -1.0;
			data->values[j+1] = -1.0;
		}

		nvmlMemory_t mem_info;
		ret = nvmlDeviceGetMemoryInfo(*devices[i], &mem_info);
		if(ret == NVML_SUCCESS) {
		/* the value is percent of allocated FB memory */
			data->values[j+2] = (float) mem_info.used * 100.0 / mem_info.total;
		} else
			data->values[j+2] = -1.0;

		unsigned int bytes_per_20ms;
		ret = nvmlDeviceGetPcieThroughput(*devices[i], NVML_PCIE_UTIL_TX_BYTES, &bytes_per_20ms);
		if (ret == NVML_SUCCESS) {
		/* Returns #bytes during a 20msec interval. Transform to bytes/sec */
			data->values[j+3] = (float) bytes_per_20ms / 0.020;
		} else
			data->values[j+3] = -1.0;

		ret = nvmlDeviceGetPcieThroughput(*devices[i], NVML_PCIE_UTIL_RX_BYTES, &bytes_per_20ms);
		if (ret == NVML_SUCCESS) {
		/* Returns #bytes during a 20msec interval. Transform to bytes/sec */
			data->values[j+4] = (float) bytes_per_20ms / 0.020;
		} else
			data->values[j+4] = -1.0;

		unsigned int tempC;
		ret = nvmlDeviceGetTemperature(*devices[i], NVML_TEMPERATURE_GPU, &tempC);
		if (ret == NVML_SUCCESS) {
		/* the value is temperature of the device, in degree */
			data->values[j+5] = (float) tempC * 1.0;
		} else
			data->values[j+5] = -1.0;

		unsigned int power_mW;
		ret = nvmlDeviceGetPowerUsage(*devices[i], &power_mW);
		if (ret == NVML_SUCCESS) {
			/* the value is power of the device in Milliwatts */
			data->values[j+6] = (float) power_mW * 1.0;
		} else
			data->values[j+6] = -1.0;
		j = j + 7;
	}
	return SUCCESS;
}

/** @brief Formats the sampling data into a json string
 *
 *  json string contains: plugin name, timestamps, metrics_name and metrics_value
 *
 */
void mf_NVML_to_json(Plugin_metrics *data, char **events, size_t num_events, char *json)
{
	struct timespec timestamp;
	char tmp[128] = {'\0'};
	char *sub_part;
	int i, ii;
	/*
	* prepares the json string, including current timestamp, and name of the plugin
	*/
	sprintf(json, "\"type\":\"NVML\"");
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
			sub_part = strstr(data->events[ii], ":");
			sub_part++;
			if(strcmp(events[i], sub_part) == 0 && (data->values[ii] >= 0.0)) {
				sprintf(tmp, ",\"%s\":%.3f", data->events[ii], data->values[ii]);
				strcat(json, tmp);
			}
		}
	}
}

/** @brief Stops the plugin
 *
 *  This methods stops papi counters gracefully;
 */
void mf_NVML_shutdown()
{
	int i;
	for(i = 0; i < devices_count; i++)
		free(devices[i]);
	free(devices);
	nvmlShutdown();
}

/* Checks if all events are not valid; return 1 when all events are not valid; 0 otherwise. */
int events_are_all_not_valid(char **events, size_t num_events) 
{
	int i, ii, counter;
	counter = 0; 
	for (i=0; i < num_events; i++) {
		for (ii = 0; ii < NVML_EVENTS_NUM; ii++) {
			/* if events name matches, counter is incremented by 1 */
			if(strcmp(events[i], NVML_metrics[ii]) == 0)
				counter++;
		}
	}
	if (counter == 0) {
		fprintf(stderr, "6-Wrong given metrics.\nPlease given metrics ");
		for (ii = 0; ii < NVML_EVENTS_NUM; ii++)
			fprintf(stderr, "%s ", NVML_metrics[ii]);
		fprintf(stderr, "\n");
		return 1;
	} else {
		return 0;
	}
}

/* initialize the nvml library */
static int load_nvml_library()
{
	nvmlReturn_t ret = nvmlInit();
	if(ret != NVML_SUCCESS) {
		fprintf(stderr, "mf_NVML_init failed during loading nvml library.\n");
		return FAILURE;
	}
	return SUCCESS;
}
