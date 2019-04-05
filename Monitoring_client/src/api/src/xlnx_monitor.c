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
// #include <stdbool.h>
#include "xlnx_monitor.h"
#include "mf_util.h"
#include "mf_api.h"
#define SUCCESS 0
#define FAILURE 1

#define false ( 1 == 0 )
#define true ( ! false )

// //entries for the ZC906
#define xilinx_path "/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/mux_device/channel-7/8-0065/hwmon/hwmon0/%s%s%s"
#define xilinx_path_fake "~/fake_fpga/%s%s%s"

#define MAX_CHANNELS 5
const char channel_labels[MAX_CHANNELS][16] = {//also referrred as Rails
	"VCCINT","VCCAUX", "VCC1V5_PL", "VADJ_FPGA", "VCC3V3_FPGA"};
/*******************************************************************************
* Implementaion
******************************************************************************/
int xlnx_stats_read(struct xlnx_stats_t *stats) {
	unsigned long long int temp;
	FILE *fp;
	char filename[128], line[256];
	//name
	sprintf(filename, xilinx_path, "name","","");
	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Warning: Could not open sensored datafile %s.\n", filename);
		return FAILURE;
// 		fprintf(stderr, "Trying with the version for testing...\n");
		sprintf(filename, xilinx_path_fake, "name","","");
		if ((fp = fopen(filename, "r")) == NULL) {
// 			fprintf(stderr, "ERROR: Could not open file %s.\n", filename);
			return FAILURE;
		}
	}
	fgets(stats->name, 30, fp);
	fclose(fp);
	//temp
	sprintf(filename, xilinx_path, "temp1_input","","");
	if ((fp = fopen(filename, "r")) == NULL) {
// 		fprintf(stderr, "Warning: Could not open sensored datafile %s.\n", filename);
// 		fprintf(stderr, "Trying with the version for testing...\n");
		sprintf(filename, xilinx_path_fake, "temp1_input","","");
		if ((fp = fopen(filename, "r")) == NULL) {
// 			fprintf(stderr, "ERROR: Could not open file %s.\n", filename);
			return FAILURE;
		}
	}
	fgets(line, 256, fp);
	sscanf(line, "%llu", &temp);
	stats->temp= temp ;
	fclose(fp);
	stats-> total_watts =0;
	char num[2];
	num[1]='\0';
	for (int i=0;i<MAX_CHANNELS;i++){
		num[0]=48+i+1;
		//channel i (VCCINT,VCCAUX, VCC1V5_PL, VADJ_FPGA, VCC3V3_FPGA : current
		sprintf(filename, xilinx_path, "curr",num,"_input"); 
		if ((fp = fopen(filename, "r")) == NULL) {
// 			fprintf(stderr, "Warning: Could not open sensored datafile %s.\n", filename);
	// 		fprintf(stderr, "Trying with the version for testing...\n");
			sprintf(filename, xilinx_path_fake, "curr",num,"_input");
			if ((fp = fopen(filename, "r")) == NULL) {
	// 			fprintf(stderr, "ERROR: Could not open file %s.\n", filename);
				return FAILURE;
			}
		}

		fgets(line, 256, fp);
		sscanf(line, "%lli", &stats->current[i]);
		fclose(fp);
		//channel 1: voltage
		sprintf(filename, xilinx_path, "in",num,"_input");
		if ((fp = fopen(filename, "r")) == NULL) {
// 			fprintf(stderr, "Warning: Could not open sensored datafile %s.\n", filename);
	// 		fprintf(stderr, "Trying with the version for testing...\n");
			sprintf(filename, xilinx_path_fake, "in",num,"_input");
			if ((fp = fopen(filename, "r")) == NULL) {
	// 			fprintf(stderr, "ERROR: Could not open file %s.\n", filename);
				return FAILURE;
			}
		}
		fgets(line, 256, fp);
		sscanf(line, "%lli", &stats->volt[i]);
		fclose(fp);
		stats-> total_watts += stats->current[i] * stats->volt[i];
	}
	return SUCCESS;
}

int xlnx_monitor(char *DataPath, long sampling_interval) {
	struct timespec timestamp;
	double timestamp_ms;
	_Bool avg_current_overflow[5]= {false,false,false,false,false};
	_Bool avg_volt_overflow[5]={false,false,false,false,false};
	_Bool avg_temp_overflow=false;
	long int counter=1;
	xlnx_stats result;
	/*initialize the values in result */
	if(xlnx_stats_read(&result)==FAILURE){
		return 0;
	}
	result.avg_temp=result.temp;
	result.max_temp=result.temp;
	result.min_temp=result.temp;
	for (int i=0;i<MAX_CHANNELS;i++){
		result.max_current[i]=result.current[i];
		result.min_current[i]=result.current[i];
		result.avg_current[i]=result.current[i];
		result.avg_volt[i]=result.volt[i];
		result.max_volt[i]=result.volt[i];
		result.min_volt[i]=result.volt[i];
	}
	/*create and open the file*/
	char FileName[256] = {'\0'};
	sprintf(FileName, "%s/%s", DataPath, METRIC_NAME_2);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		return 0;
	}
	/*in a loop do data sampling and write into the file*/
	while(running) {
		usleep(sampling_interval * 1000);
		if(xlnx_stats_read(&result)==FAILURE){
			fclose(fp);
			return 0;
		}
		/*get current timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp);
		timestamp_ms = timestamp.tv_sec * 1000.0 + (double)(timestamp.tv_nsec / 1.0e6);
		/*calculate the values for disk stats */
		fprintf(fp, "\"local_timestamp\":%.1f", timestamp_ms);
		fprintf(fp, ",\"temperature\":%.2f", (float) result.temp/1000.0);
		for (int i=0;i<MAX_CHANNELS;i++){
		//channel i (VCCINT,VCCAUX, VCC1V5_PL, VADJ_FPGA, VCC3V3_FPGA : current
			fprintf(fp, ",\"%s%s\":%lli", channel_labels[i],"_current", result.current[i]);
			fprintf(fp, ",\"%s%s\":%lli", channel_labels[i],"_volt", result.volt[i]);
			if(result.max_current[i]<result.current[i]) result.max_current[i]=result.current[i];
			if(result.min_current[i]>result.current[i]) result.min_current[i]=result.current[i];
			if(result.max_volt[i]<result.volt[i]) result.max_volt[i]=result.volt[i];
			if(result.min_volt[i]>result.volt[i]) result.min_volt[i]=result.volt[i];
			if(avg_current_overflow[i]== false)
				result.avg_current[i]=ladd(result.avg_current[i],result.current[i],&avg_current_overflow[i]);
			if(avg_volt_overflow[i]== false)
				result.avg_volt[i]=ladd(result.avg_volt[i],result.volt[i],&avg_volt_overflow[i]);
		}
		fprintf(fp, "\n");
		if(result.max_temp<result.temp) result.max_temp=result.temp;
		if(result.min_temp>result.temp) result.min_temp=result.temp;
		if(avg_temp_overflow== false)
			result.avg_temp=ladd(result.avg_temp,result.temp,&avg_temp_overflow);
		counter++;
	}
	fclose(fp);

	sprintf(FileName, "%s/stats_%s", DataPath, METRIC_NAME_2);
	fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		return 0;
	}
	fprintf(fp, "\"local_timestamp\":%.1f", timestamp_ms);
	fprintf(fp, ",\"%s\":%.3f", "total_watts",result.total_watts );
	fprintf(fp, "," );
	for (int i=0;i<MAX_CHANNELS;i++){
		fprintf(fp, "\"stats_%s%s\":{", channel_labels[i],"_current" );
		fprintf(fp, "\"count\":%li", counter);
		fprintf(fp, ",\"min\":%lli", result.min_current[i]);
		fprintf(fp, ",\"max\":%lli", result.max_current[i]);
		if(avg_current_overflow[i]== false){
			fprintf(fp, ",\"avg\":%lli", result.avg_current[i]/counter);
			fprintf(fp, ",\"sum\":%lli}", result.avg_current[i]);
		}else{
			fprintf(fp, ",\"avg\":%s", "overflow");
			fprintf(fp, ",\"sum\":%s}", "overflow");
		}

		fprintf(fp, ",\"stats_%s%s\":{", channel_labels[i],"_volt" );
		fprintf(fp, "\"count\":%li", counter);
		fprintf(fp, ",\"min\":%lli", result.min_volt[i]);
		fprintf(fp, ",\"max\":%lli", result.max_volt[i]);
		if(avg_volt_overflow[i]== false){
			fprintf(fp, ",\"avg\":%lli", result.avg_volt[i]/counter);
			fprintf(fp, ",\"sum\":%lli}", result.avg_volt[i]);
		}else{
			fprintf(fp, ",\"avg\":\"%s\"", "overflow");
			fprintf(fp, ",\"sum\":\"%s\"}", "overflow");
		}
		fprintf(fp, "," );
	}
	fprintf(fp, "\"stats_temperature\":{"  );
	fprintf(fp, "\"count\":%lu", counter);
	fprintf(fp, ",\"min\":%.2f", (float) result.min_temp/1000.0);
	fprintf(fp, ",\"max\":%.2f", (float) result.max_temp/1000.0);
	if(avg_temp_overflow== false){
		fprintf(fp, ",\"avg\":%.2f", (float) result.avg_temp/(1000.0*counter));
		fprintf(fp, ",\"sum\":%.2f}", (float) result.avg_temp/1000.0);
	}else{
		fprintf(fp, ",\"avg\":\"%s\"", "overflow");
		fprintf(fp, ",\"sum\":\"%s\"}", "overflow");
	}
	fprintf(fp, "\n" );
	fclose(fp);
	printf("finishing files\n\n");
	return 1;
}
