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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "dummy_monitor.h"
#include "mf_api.h"

/*******************************************************************************
 * Implementaion
 ******************************************************************************/
int dummy_get_stats(int pid, dummy_stats *dummy_info) {
	/*update the before data */
	dummy_info->data_before = dummy_info->data_after;
	dummy_info->data_after= rand() % 100;
	return 1;
}

int dummy_monitor(int pid, char *DataPath, long sampling_interval){
	/*create and open the file*/
	char FileName[256] = {'\0'};
	sprintf(FileName, "%s/%s", DataPath, METRIC_NAME_5);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		return 0;
	}
	struct timespec timestamp;
	double timestamp_ms;
	float variation,throughput;
	dummy_stats result;
	/*initialize the values in result */
	result.data_after = 0;
	dummy_get_stats(pid, &result);
	/*in a loop do data sampling and write into the file*/
	while(running) {
		usleep(sampling_interval * 1000);
		dummy_get_stats(pid, &result);
		/*get current timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp);
    	timestamp_ms = timestamp.tv_sec * 1000.0  + (double)(timestamp.tv_nsec / 1.0e6);
    	/*calculate the values for some stats */
    	variation = result.data_after - result.data_before;
    	throughput = (variation) * 1000.0 / sampling_interval; //in bytes/s
    	fprintf(fp, "\"local_timestamp\":\"%.1f\", \"%s\":%.3f\n", timestamp_ms, "dummy_throughput", throughput);
	}
	/*close the file*/
	fclose(fp);
	return 1;
}
