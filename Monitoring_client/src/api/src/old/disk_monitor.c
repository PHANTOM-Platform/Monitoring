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
#include "disk_monitor.h"
#include "mf_api.h"
#define SUCCESS 0
#define FAILURE 1
/*******************************************************************************
* Implementaion
******************************************************************************/
int disk_stats_read(int pid, disk_stats *disk_info) {
	FILE *fp;
	char filename[128], line[256];
	/*update the before read/write bytes */
	disk_info->read_bytes_before = disk_info->read_bytes_after;
	disk_info->write_bytes_before = disk_info->write_bytes_after;
	sprintf(filename, "/proc/%d/io", pid);
	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "ERROR: Could not open file %s.\n", filename);
		return FAILURE;
	}
	while (fgets(line, 256, fp) != NULL) {
		if (!strncmp(line, "read_bytes:", 11))
			sscanf(line + 12, "%llu", &disk_info->read_bytes_after);
		if (!strncmp(line, "write_bytes:", 12))
			sscanf(line + 13, "%llu", &disk_info->write_bytes_after);
	}
	fclose(fp);
	return SUCCESS;
}

int disk_monitor(int pid, char *DataPath, long sampling_interval) {
	/*create and open the file*/
	char FileName[256] = {'\0'};
	sprintf(FileName, "%s/%s", DataPath, METRIC_NAME_2);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		return 0;
	}
	struct timespec timestamp;
	double timestamp_ms;
	unsigned long long read, write;
	float throughput;
	disk_stats result;
	/*initialize the values in result */
	result.read_bytes_after = 0;
	result.write_bytes_after = 0;
	disk_stats_read(pid, &result);
	/*in a loop do data sampling and write into the file*/
	while(running) {
		usleep(sampling_interval * 1000);
		disk_stats_read(pid, &result);
		/*get current timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp);
		timestamp_ms = timestamp.tv_sec * 1000.0  + (double)(timestamp.tv_nsec / 1.0e6);
		/*calculate the values for disk stats */
		read = result.read_bytes_after - result.read_bytes_before;
		write = result.write_bytes_after - result.write_bytes_before;
		throughput = (read + write) * 1000.0 / sampling_interval; //in bytes/s
		fprintf(fp, "\"local_timestamp\":\"%.1f\", \"%s\":%llu, \"%s\":%llu, \"%s\":%.3f\n", timestamp_ms,
			"disk_read", read,
			"disk_write", write,
			"disk_throughput", throughput);
	}
	fclose(fp);
	return 1;
}
