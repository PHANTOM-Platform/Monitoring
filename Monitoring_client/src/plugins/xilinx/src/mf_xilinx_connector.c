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
#include "mf_xilinx_connector.h"

#define SUCCESS 0
#define FAILURE 1

	const char xBLUE[]="\033[0;34m";
	const char xLIGHT_BLUE[]="\033[1;34m";
	const char xyellow[]="\033[1;33m";
	const char xWHITE[]="\033[1;37m";
	const char xNO_COLOUR[]="\033[0m";

	
//Texas Instruments UCD chips are "Programmable Digital Power Controllers"
//This definition of UCD_PATH is for the ZC706, it will differ on other Xilinx Boards !!!
#define UCD_PATH "/home/jmontana/fake_fpga/"
// #define UCD_PATH "/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/mux_device/channel-7/8-0065/hwmon/hwmon0"
#define HAS_TEST1_STAT 0x01

/*******************************************************************************
 * Variable Declarations
 ******************************************************************************/
/* flag indicates which events are given as input */
unsigned int flag = 0;
/* time in seconds */
double before_time, after_time;

#define RESOURCES_EVENTS_NUM 2
const char xilinx_supported_metrics[RESOURCES_EVENTS_NUM][32] = { "test1", "test2" };

/*******************************************************************************
 * Functions implementation
 ******************************************************************************/
/* Adds events to the data->events, if the events are valid */
int flag_init(char **events, size_t num_events) {
	int i, ii;
	for (i=0; i < num_events; i++) {
		for (ii = 0; ii < RESOURCES_EVENTS_NUM; ii++) {
			/* if events name matches */
			if(strcmp(events[i], xilinx_supported_metrics[ii]) == 0) {
				/* get the flag updated */
				unsigned int current_event_flag = 1 << ii;
				flag = flag | current_event_flag;
			}
		}
	}
	if (flag == 0) {
		fprintf(stderr, "7-Wrong given metrics.\nPlease given metrics ");
		for (ii = 0; ii < RESOURCES_EVENTS_NUM; ii++)
			fprintf(stderr, "%s ", xilinx_supported_metrics[ii]);
		fprintf(stderr, "\n");
		return FAILURE;
	} else 
		return SUCCESS;
}

/** @brief Initializes the xilinx plugin
 *
 *  Check if input events are valid; add valid events to the data->events
 *  acquire the previous value and before timestamp
 *
 *  @return 1 on success; 0 otherwise.
 */
int mf_xilinx_init(Plugin_metrics *data, char **events, size_t num_events) {
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

/* Gets ram usage rate (unit is %); return the ram usage rate on success; 0.0 otherwise */
int test1_read() {
	int test1_rate = 0.0;
	test1_rate = rand() % 100;
	return test1_rate;
}


/* Gets the voltage, current and temperature */
int file_read_float(float *value, const char filename[]) {
	FILE *fp;
	char line[1024];
	*value = 0;
	char file_full_path[1024];
	strcpy(file_full_path,UCD_PATH);
	strcat(file_full_path,filename);
	fp = fopen(file_full_path, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", file_full_path);
		return FAILURE;
	}
	if (fgets(line, 1024, fp) != NULL)
		sscanf(line, "%f", value);
	fclose(fp);
	return SUCCESS;
}

int file_read_string(char value[], char filename[]) {
	FILE *fp;
	char line[1024];
	*value = 0;
	char file_full_path[1024];
	strcpy(file_full_path,UCD_PATH);
	strcat(file_full_path,filename);
	fp = fopen(file_full_path, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error: Cannot open %s.\n", file_full_path);
		return FAILURE;
	}
	if (fgets(line, 1024, fp) != NULL)
		sscanf(line, "%s", value);
	fclose(fp);
	return SUCCESS;
}

char* itoa(int i, char b[]){
	char const digit[] = "0123456789";
	char* p = b;
	if(i<0){
		*p++ = '-';
		i *= -1;
	}
	int shifter = i;
	do{ //Move to where representation ends
		++p;
		shifter = shifter/10;
	}while(shifter);
	*p = '\0';
	do{ //Move back, inserting digits as u go
		*--p = digit[i%10];
		i = i/10;
	}while(i);
	return b;
}

int print_xlnx_report(Plugin_metrics *data) {
	float a;
	float watts=0;	
// #### GLOBAL consts and Variables:
	#define NUMBER_OF_STRINGS 5
	#define STRING_LENGTH 17	
	const char labels[NUMBER_OF_STRINGS][STRING_LENGTH+1] = {"VCCINT     ", "VCCAUX     ", "VCC1V5_PL  ", "VADJ_FPGA  ", "VCC3V3_FPGA"}; //also referred as Rail
	const float expected_v[NUMBER_OF_STRINGS] = {1.000, 1.800, 1.500, 2.500, 3.300};
// 	char range_a[NUMBER_OF_STRINGS][STRING_LENGTH+1] = {"0-16", "0-4.2", "0-2.8", "0-1", "0-0.5"};
//	#range_a[NUMBER_OF_STRINGS][STRING_LENGTH+1] = {"0-8A" "0-4A" "0-2A" "0-2A" "0-2A");
// 	printf("\n\n\n%sMetrics from board %sZC706%s, where power is controlled by %s%s%s\n",BLUE,WHITE,BLUE,yellow,xilinx_info.name,NO_COLOUR); // expected name UCD90120A
	printf("\n\n\n%sMetrics from board %sZC706%s\n",xBLUE,xWHITE,xNO_COLOUR); // expected name UCD90120A
	printf("%sTEMPERATURE of power controller: %s%.2fC%s\n",xBLUE,xyellow,data->values[0] /1000.0,xNO_COLOUR);
	printf("\n%sChannel_num  Rail       Expected      Measured      Measured       Power\n",xBLUE);
	printf("%s                        Voltage [V]   Voltage [mV]  current [mA]   Consumption [mW]\n",xBLUE);
	printf("------------------------------------------------------------------------------------\n");
	for (int i=0;i<NUMBER_OF_STRINGS;i++){ // this is the counter of channels
		a= (data->values[2*i+1] * data->values[2*i] )/1000.0;
		printf("%s   %i        %s%s%s   %.3f%s         %4.0f            %4.0f           %7.2f%s\n",xBLUE,i+1,xWHITE,labels[i],xBLUE,expected_v[i],xyellow,data->values[2*i+1], data->values[2*i], a,xNO_COLOUR);
		watts+=a;
	}
	printf("\n\n%sTOTAL power consumption: %s%.2f mW%s\n\n",xBLUE,xyellow,watts,xNO_COLOUR);
	/* update timestamp */
	//before_time = after_time;
	return SUCCESS;
}

//         "TaskID" : "pthread-example",
//         "type" : "xlnx_monitor",
//         "host" : "node01",
//         "local_timestamp" : "2019-01-03T22:11:35.288",
//         "temperature" : "22.53",
//         "VCCINT_current" : "125",
//         "VCCINT_volt" : "1000",
//         "VCCAUX_current" : "62",
//         "VCCAUX_volt" : "1777",
//         "VCC1V5_PL_current" : "46",
//         "VCC1V5_PL_volt" : "1496",
//         "VADJ_FPGA_current" : "93",
//         "VADJ_FPGA_volt" : "2449",
//         "VCC3V3_FPGA_current" : "46",
//         "VCC3V3_FPGA_volt" : "3291",
//         "server_timestamp" : "2019-01-03T22:11:37.268"



/** @brief Samples all possible events and stores data into the Plugin_metrics
 *
 *  @return SUCCESS on success; FAILURE otherwise.
 */
int mf_xilinx_sample(Plugin_metrics *data) {
	char new_string[20];
	char filename[1024];

	/* get current timestamp in second */
	struct timespec timestamp;
	int i;
	
	for (i = 0; i < data->num_events; i++)
		data->values[i] = 0.0;
	
	clock_gettime(CLOCK_REALTIME, &timestamp);
	after_time = timestamp.tv_sec * 1.0  + (double)(timestamp.tv_nsec / 1.0e9);
	//double time_interval = after_time - before_time;

	i = 0;
// #### NAME AND TEMPERATURE
// 	file_read_string(xilinx_info.name,"name");
// 	if(flag & HAS_TEST1_STAT) {
		file_read_float(&data->values[i],"temp1_input");
		i++;
// 	}
	for (int j=0;j<5;j++){ // this is the counter of channels
		strcpy(filename,"curr");
		strcat(filename, itoa((int) j+1, new_string));
		strcat(filename,"_input");
		file_read_float(&data->values[i],filename); i++;
		strcpy(filename,"in");
		strcat(filename, itoa((int) j+1, new_string));
		strcat(filename,"_input");
		file_read_float(&data->values[i],filename); i++;
	}
	// ### REPORT
	print_xlnx_report(data);
	return SUCCESS;
}



/** @brief Formats the sampling data into a json string
 *
 *  json string contains: plugin name, timestamps, metrics_name and metrics_value
 *
 */
void mf_xilinx_to_json(Plugin_metrics *data, char *json) {
    char tmp[128] = {'\0'};
    int i;
    /*
     * prepares the json string, including current timestamp, and name of the plugin
     */
    sprintf(json, "\"type\":\"xilinx\"");
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
