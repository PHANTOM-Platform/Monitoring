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

/****************************************************************************
|* This code uses the NVIDIA Management Library (NVML) which is subject to *|
|* NVIDIA ownership rights under U.S. and international Copyright laws. In *|
|* the best of our understanding, users of NVML are granted a nonexclusive,*|
|* royalty-free license to use it in individual and commercial software.   *|
|* *|
|* NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF NVML FOR        *|
|* ANY PURPOSE. IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR                  *|
|* IMPLIED WARRANTY OF ANY KIND. NVIDIA DISCLAIMS ALL WARRANTIES WITH      *|
|* REGARD TO NVML, INCLUDING ALL IMPLIED WARRANTIES OF                     *|
|* MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR          *|
|* PURPOSE. IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL,            *|
|* INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES          *|
|* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN      *|
|* AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING     *|
|* OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE      *|
|* CODE OR THE USE OF NVML.                                                *|
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "nvml_monitor.h"
#include "mf_api.h"
#include <malloc.h>
#include <nvml.h>
#include <math.h>

//	const char  BLUE[]="\033[0;34m";
//	const char    LIGHT_GRAY[]="\033[0;37m";
// 	const char   LIGHT_GREEN[]="\033[1;32m";
	const char    LIGHT_BLUE[]="\033[1;34m";
//	const char    LIGHT_CYAN[]="\033[1;36m";
//	const char        yellow[]="\033[1;33m";
//	const char         WHITE[]="\033[1;37m";
//	const char           RED[]="\033[0;31m";
//	const char        marron[]="\033[2;33m";
	const char     NO_COLOUR[]="\033[0m";
//	const char        white[]="\033[0;0m";
/*******************************************************************************
* String functions
******************************************************************************/
// reverses a string 'str' of length 'len'
void mreverse(char *str, int len) {
    int i=0, j=len-1, temp;
    while (i<j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}
char myBuffer[255];
char myfBuffer[255];
// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
// returns the string
const char* mintToStr(const int x, const int d) {
    int i = 0;
	int temp=x;
	if(temp==0) myBuffer[i++] = '0';
    while (temp) {
        myBuffer[i++] = (temp%10) + '0';
        temp = temp/10;
    }
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        myBuffer[i++] = '0';
    mreverse(myBuffer, i);
    myBuffer[i] = '\0';
    return myBuffer;
}

const char* llintToStr(const long long int x, const int d) {
    int i = 0;
	long long int temp=x;
	if(temp==0) myBuffer[i++] = '0';
    while (temp) {
        myBuffer[i++] = (temp%10) + '0';
        temp = temp/10;
    }
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        myBuffer[i++] = '0';
    mreverse(myBuffer, i);
    myBuffer[i] = '\0';
    return myBuffer;
}

/** mftoa(n, res, afterpoint)
* n          --> Input Number
* res[]      --> Array where output string to be stored
* afterpoint --> Number of digits to be considered after point.
* 
* For example mftoa(1.555, str, 2) should store "1.55" in res and
* mftoa(1.555, str, 0) should store "1" in res.*/
// Converts a floating point number to string.
const char *mftoa(const float n, const int afterpoint) {
    // Extract integer part
    int ipart = (int)n; 
	
    // Extract floating part
    float fpart = n - (float)ipart;
    // convert integer part to string
    strcpy(myfBuffer,mintToStr(ipart, 0));
	int i=strlen(myfBuffer);
    // check for display option after point
    if (afterpoint != 0) {
        myfBuffer[i] = '.';  // add dot
        myfBuffer[i+1] = '\0';  // add dot
        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);
        strcat(myfBuffer,mintToStr((int)fpart, afterpoint));
    }
	return myfBuffer;
}

//Function for increase dynamically a string concatenating strings at the end
//It free the memory of the first pointer if not null
// returns: s1 <- s1 + s2 + s3 + s4
char* concat_strings(char **s1, const char *s2, const char *s3, const char *s4){
	char *result = NULL;
	unsigned int new_lenght= strlen(s2)+strlen(s3)+strlen(s4)+1; //+1 for the null-terminator;
	if(*s1 != NULL){
		new_lenght+= strlen(*s1);//current lenght
		if(new_lenght> malloc_usable_size(*s1)){
			result = (char *) malloc(new_lenght);
			if(result==NULL) {
				printf("Failed to allocate memory.\n");
				exit(1);
			}
			strcpy(result, *s1);
			free(*s1);
		}else{
			result = *s1;
		}
	}else{
		result = (char *) malloc(new_lenght);
		if(result==NULL) {
			printf("Failed to allocate memory.\n");
			exit(1);
		}
		result[0]='\0';
	}
	*s1 = result;
	strcat(result, s2);
	strcat(result, s3);
	strcat(result, s4);
	return result;
}

/*******************************************************************************
* Function Definitions
******************************************************************************/

void report_metrics(int i, struct nvml_stats *nvml_info){
	int j;
// 	char *metrics=NULL;
// 	concat_strings()
//	printf("%s----- DEVICE num %d -----%s\n", i, LIGHT_BLUE, NO_COLOUR);
	printf("{\n");
	printf(" \"device_name\":\"%s\",\n", nvml_info->name);
	printf(" \"pci-bus\":\"%s\",\n", nvml_info->pcibusId);
	//=========================================== TEMPERATURE ===========================================
	printf("\n%s----- Temperature (Celsius degrees)-----%s\n",LIGHT_BLUE, NO_COLOUR);
	if(nvml_info->valid_temperature==-1)
		printf(" \"temp_notice\"：\"Device %i NVML_TEMPERATURE_GPU Failed: %s\n", i, nvml_info->temperatureErrorString);
	else
		printf(" \"temperature\":%d,\n", nvml_info->temperature);
	//===========================================GPU LOAD===========================================
	printf("\n%s----- GPU Usage rate (percentaje %%) -----%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_utilization==-1) {
		printf(" \"utilization_notice\"：\"Device %i nvmlDeviceGetUtilizationRates Failed : %s\n", i, nvml_info->utilizationErrorString);
	} else {
		printf(" \"gpu_usage_rate\":%lld,\n", nvml_info->utilizationgpu);
		printf(" \"memory_usage\":%lld,\n", nvml_info->utilizationmemory);
	}
	//===========================================Power Consumption===========================================
	printf("\n%s----- Power Consumption (mW)-----%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_power==-1)
		printf(" \"gpu_power_consumtion_notice\":\"nvmlDeviceGetPowerUsage Failed\",\n");//nvml_info->powerErrorString);
	else
		printf(" \"gpu_power_consumtion\":%u,\n", nvml_info->power);
	//===========================================Frame Buffer memory===========================================
	printf("\n%s------ On-board Frame Buffer(FB) memory (bytes)-------%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_memory != 0) {
		printf(" \"fb_memory_notice\"：\"nvmlDeviceGetMemoryInfo Failed\",\n");// nvml_info->memoryErrorString);
	} else {
		printf(" \"total_installed_fb_memory\"：%lld,\n", nvml_info->memorytotal);
		printf(" \"unallocated_fb_memory\"：%lld,\n", nvml_info->memoryfree);
		printf(" \"allocated_fb_memory\"：%lld,\n", nvml_info->memoryused);
	}
	//==========================================BAR1 MEMORY ======================================
	printf("\n%s------ BAR1 memory (is used to map the FB (device memory) so that it can be directly accessed by the CPU or by 3rd party devices)) (bytes)-------%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_bar1Memory != 0) {
		printf(" \"bar1_memory_notice\"：\"nvmlDeviceGetBAR1MemoryInfo Failed\",\n");// nvml_info->bar1MemoryErrorString);
	} else {
		printf(" \"total bar1_memory\"：%lld,\n", nvml_info->bar1MemoryTotal);
		printf(" \"unallocated_bar1_memory\"：%lld,\n", nvml_info->bar1MemoryFree);
		printf(" \"allocated_bar1_memory\"：%lld,\n", nvml_info->bar1MemoryUsed);
	}
	printf("\n%s------ Clocks (MHz)-------%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_Graphicsclock != 0)
		printf(" \"graphics_clk_notice\"：\"Failed to get NVML_CLOCK_GRAPHICS info\",\n");//,nvml_info->GraphicsclockErrorString);
	else
		printf(" \"graphics_clk\"：%d,\n \"max_graphics_clk\"：%d,\n", nvml_info->Graphicsclock, nvml_info->Graphicsmax_clock);
	if (nvml_info->valid_SMclock != 0)
		printf(" \"sm_clk_notice\":\"Failed to get NVML_CLOCK_SM info\",\n");//nvml_info->SMclockErrorString);
	else
		printf(" \"sm_clk\":%d,\n \"max_sm_clk\"：%d,\n", nvml_info->SMclock, nvml_info->SMmax_clock);
	if (nvml_info->valid_Memoryclock != 0)
		printf(" \"memory_clk_notice\":\"Failed to get NVML_CLOCK_MEM info\",\n");//nvml_info->MemoryclockErrorString);
	else
		printf(" \"memory_clk\"：%d,\n \"max_memory_clk\"：%d,\n", nvml_info->Memoryclock, nvml_info->Memorymax_clock);
	if (nvml_info->valid_Videoclock != 0)
		printf(" \"video_clk_notice\"：\"Failed to get NVML_CLOCK_VIDEO info\",\n");// nvml_info->VideoclockErrorString);
	else
		printf(" \"video_clk\"：%d,\n \"video_clk_max\"：%d,\n", nvml_info->Memoryclock, nvml_info->Videomax_clock);
	//==========================================  Information of PCIe utilization counters ========
	printf("\n%s------ PCIe utilization (bytes/sec) -------%s\n",LIGHT_BLUE, NO_COLOUR); 
	if (nvml_info->valid_pcie_tx==0)
		printf(" \"pci_tx\"：%.0f,\n", nvml_info->pcie_tx);
	else
		printf(" \"pci_tx_notice\"：\"Failed to get nvmlDeviceGetPcieThroughput info\",\n");//,nvml_info->pcie_tx_ErrorString);
	if (nvml_info->valid_pcie_rx==0)
		printf(" \"pci_rx\"：%.0f,\n", nvml_info->pcie_rx);
	else
		printf(" \"pci_rx_notice\"：\"Failed to get nvmlDeviceGetPcieThroughput info\",\n");//,nvml_info->pcie_rx_ErrorString);
	//========================================== INFO OF THE RUNNING PROCESSES IN THE GPU ======================================
	printf("\n%s------ Information of the MEMORY used by the processes running on the GPU ------- (bytes)%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_ProcessCount != 0) {
		printf("  \"total_num_processes_notice\":\"Failed to get ComputeRunningProcesses\",\n");// nvml_info->ProcessCountErrorString);
	}else{
		printf(" \"total_num_processes\":%i",nvml_info->ProcessCount);
		if (nvml_info->ProcessCount>0) printf(",");
		printf("\n");
		for (j = 0; j < nvml_info->ProcessCount; j++){
			printf(" \nprocess_%i\":",j);
			printf("{\"pid\"： %d,\n  \"memory_consumption\"：%lld}", nvml_info->metric_gpu_processes[j]->pid, nvml_info->metric_gpu_processes[j]->usedGpuMemory);
			if (j+1 < nvml_info->ProcessCount) printf(",");
			printf("\n");
		}
	}
	printf("}\n");
}



void get_json_metrics(int i, struct nvml_stats *nvml_info, char **metrics_json){
	int j;
	concat_strings(metrics_json, " \"device_name\":\"", nvml_info->name, "\",\n");
	concat_strings(metrics_json, " \"pci-bus\":\"", nvml_info->pcibusId,"\",\n");
	//=========================================== TEMPERATURE ===========================================
// 	printf("\n%s----- Temperature (Celsius degrees)-----%s\n",LIGHT_BLUE, NO_COLOUR);
	if(nvml_info->valid_temperature==-1)
		concat_strings(metrics_json, " \"temp_notice\"：\"NVML_TEMPERATURE_GPU Failed: ", nvml_info->temperatureErrorString, "\",\n");
	else
		concat_strings(metrics_json, " \"temperature\":", mintToStr(nvml_info->temperature,0), ",\n");
	//===========================================GPU LOAD===========================================
// 	printf("\n%s----- GPU Usage rate (percentaje %%) -----%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_utilization==-1) {
		concat_strings(metrics_json, " \"utilization_notice\"：\"nvmlDeviceGetUtilizationRates Failed : ", nvml_info->utilizationErrorString, "\",\n");
	} else {
		concat_strings(metrics_json, " \"gpu_usage_rate\":", llintToStr(nvml_info->utilizationgpu,0), ",\n");
		concat_strings(metrics_json, " \"memory_usage\":", llintToStr(nvml_info->utilizationmemory,0), ",\n");
	}
	//===========================================Power Consumption===========================================
// 	printf("\n%s----- Power Consumption (mW)-----%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_power==-1)
		concat_strings(metrics_json, " \"gpu_power_consumption_notice\":\"nvmlDeviceGetPowerUsage Failed\",\n","","");//nvml_info->powerErrorString);
	else
		concat_strings(metrics_json, " \"gpu_power_consumption\":", llintToStr(nvml_info->power,0), ",\n");
	//===========================================Frame Buffer memory===========================================
// 	printf("\n%s------ On-board Frame Buffer(FB) memory (bytes)-------%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_memory != 0) {
		concat_strings(metrics_json, " \"fb_memory_notice\"：\"nvmlDeviceGetMemoryInfo Failed\",\n","","");// nvml_info->memoryErrorString);
	} else {
		concat_strings(metrics_json, " \"total_installed_fb_memory\"：", llintToStr(nvml_info->memorytotal,0),",\n");
		concat_strings(metrics_json, " \"unallocated_fb_memory\"：", llintToStr(nvml_info->memoryfree,0),",\n");
		concat_strings(metrics_json, " \"allocated_fb_memory\"：", llintToStr(nvml_info->memoryused,0),",\n");
	}
	//==========================================BAR1 MEMORY ======================================
// 	printf("\n%s------ BAR1 memory (is used to map the FB (device memory) so that it can be directly accessed by the CPU or by 3rd party devices)) (bytes)-------%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_bar1Memory != 0) {
		concat_strings(metrics_json, " \"bar1_memory_notice\"：\"nvmlDeviceGetBAR1MemoryInfo Failed\",\n","","");// nvml_info->bar1MemoryErrorString);
	} else {
		concat_strings(metrics_json, " \"total bar1_memory\"：", llintToStr(nvml_info->bar1MemoryTotal,0),",\n");
		concat_strings(metrics_json, " \"unallocated_bar1_memory\"：", llintToStr(nvml_info->bar1MemoryFree,0),",\n");
		concat_strings(metrics_json, " \"allocated_bar1_memory\"：", llintToStr(nvml_info->bar1MemoryUsed,0),",\n");
	}
// 	printf("\n%s------ Clocks (MHz)-------%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_Graphicsclock != 0)
		concat_strings(metrics_json, " \"graphics_clk_notice\"：\"Failed to get NVML_CLOCK_GRAPHICS info\",\n","","");//,nvml_info->GraphicsclockErrorString);
	else{
		concat_strings(metrics_json, " \"graphics_clk\"：", mintToStr(nvml_info->Graphicsclock,0),",\n");
		concat_strings(metrics_json, " \"max_graphics_clk\"：", mintToStr(nvml_info->Graphicsmax_clock,0),",\n");
	}
	if (nvml_info->valid_SMclock != 0)
		concat_strings(metrics_json, " \"sm_clk_notice\":\"Failed to get NVML_CLOCK_SM info\",\n","","");//nvml_info->SMclockErrorString);
	else{
		concat_strings(metrics_json, " \"sm_clk\":", mintToStr(nvml_info->SMclock,0),",\n");
		concat_strings(metrics_json, " \"max_sm_clk\"：", mintToStr(nvml_info->SMmax_clock,0),",\n");
	}
	if (nvml_info->valid_Memoryclock != 0)
		concat_strings(metrics_json, " \"memory_clk_notice\":\"Failed to get NVML_CLOCK_MEM info\",\n","","");//nvml_info->MemoryclockErrorString);
	else{
		concat_strings(metrics_json, " \"memory_clk\"：", mintToStr(nvml_info->Memoryclock,0),",\n");
		concat_strings(metrics_json, " \"max_memory_clk\"：", mintToStr(nvml_info->Memorymax_clock,0),",\n");
	}
	if (nvml_info->valid_Videoclock != 0)
		concat_strings(metrics_json, " \"video_clk_notice\"：\"Failed to get NVML_CLOCK_VIDEO info\",\n","","");// nvml_info->VideoclockErrorString);
	else{
		concat_strings(metrics_json, " \"video_clk\"：", mintToStr(nvml_info->Memoryclock,0),",\n");
		concat_strings(metrics_json, " \"video_clk_max\"：", mintToStr(nvml_info->Videomax_clock,0),",\n");
	}
	//==========================================  Information of PCIe utilization counters ========
// 	printf("\n%s------ PCIe utilization (bytes/sec) -------%s\n",LIGHT_BLUE, NO_COLOUR); 
	if (nvml_info->valid_pcie_tx==0){
		concat_strings(metrics_json, " \"pci_tx\"：", mftoa(nvml_info->pcie_tx,0),",\n");
	}else
		concat_strings(metrics_json, " \"pci_tx_notice\"：\"Failed to get nvmlDeviceGetPcieThroughput info\",\n","","");//,nvml_info->pcie_tx_ErrorString);
	if (nvml_info->valid_pcie_rx==0)
		concat_strings(metrics_json, " \"pci_rx\"：", mftoa(nvml_info->pcie_rx,0),",\n");
	else
		concat_strings(metrics_json, " \"pci_rx_notice\"：\"Failed to get nvmlDeviceGetPcieThroughput info\",\n","","");//,nvml_info->pcie_rx_ErrorString);
	//========================================== INFO OF THE RUNNING PROCESSES IN THE GPU ======================================
// 	printf("\n%s------ Information of the MEMORY used by the processes running on the GPU ------- (bytes)%s\n",LIGHT_BLUE, NO_COLOUR);
	if (nvml_info->valid_ProcessCount != 0) {
		concat_strings(metrics_json, "  \"total_num_processes_notice\":\"Failed to get ComputeRunningProcesses\",\n","","");// nvml_info->ProcessCountErrorString);
	}else{
		concat_strings(metrics_json, " \"total_num_processes\":",llintToStr(nvml_info->ProcessCount,0),"\n");
		if (nvml_info->ProcessCount>0) concat_strings(metrics_json, ",","","");
		concat_strings(metrics_json, "\n","","");
		for (j = 0; j < nvml_info->ProcessCount; j++){
			concat_strings(metrics_json, " \nprocess_%i\":", mintToStr(j,0),",\n");
			concat_strings(metrics_json, "{\"pid\"：\":",mintToStr(nvml_info->metric_gpu_processes[j]->pid,0),",\n");
			concat_strings(metrics_json, "{\"memory_consumption\"：\":",llintToStr(nvml_info->metric_gpu_processes[j]->usedGpuMemory,0),",\n");
			
			if (j+1 < nvml_info->ProcessCount) concat_strings(metrics_json, ",","","");
			concat_strings(metrics_json, "\n","","");
		}
	}
}


void get_clock_info(const nvmlDevice_t device, nvmlClockType_t type, int *valid, unsigned int* clock , unsigned int* maxclock, char **error){
	nvmlReturn_t result;// it is expected to be an unsigned int betweem 0 and 999
	const char message1[]="nvmlDeviceGetMaxClockInfo Failed : ";
	const char message2[]="nvmlDeviceGetClockInfo Failed : ";
	if(*error != NULL) free(*error);
	result = nvmlDeviceGetMaxClockInfo(device, type, maxclock);
	if (NVML_SUCCESS != result){
		*valid=-1;
		int size=strlen(message1)+strlen(nvmlErrorString(result))+1;
		*error = (char*) malloc(size * sizeof(char));
		strcpy(*error, message1);
		strcat(*error, nvmlErrorString(result));
		return;
	}
	result = nvmlDeviceGetClockInfo(device, type, clock);
	if (NVML_SUCCESS != result){
		*valid=-1;
		int size=strlen(message2)+strlen(nvmlErrorString(result))+1;
		*error = (char*) malloc(size * sizeof(char));
		strcpy(*error, message2);
		strcat(*error, nvmlErrorString(result));
		return;
	}
	*error=NULL;
	*valid=0;
}

// reserves dynamically memory for the destination char string (if not reserved, or the available space is not big enough)
// and copies the input char string into the destination string
int allocate_and_asign(char **destination, const char *input, const unsigned int input_size){
	if(*destination ==NULL){
		*destination = (char*) malloc(input_size * sizeof(char));
	}else if(malloc_usable_size(*destination) < input_size){
		free(*destination);
		*destination = (char*) malloc(input_size * sizeof(char));
	}
	if(*destination == NULL) return 1;
	strcpy(*destination, input);
	return 0;
}

/*
* Return 0 if succedd, or error code in other case
*/
int nvml_get_stats(const int device_count, struct nvml_stats **nvml_info){
	int i,j;
	nvmlProcessInfo_t infos[999];//we don't know in advance how many processes are running
	nvmlReturn_t result;// it is expected to be an unsigned int betweem 0 and 999
	for (i = 0; i < device_count; i++) {
		nvmlDevice_t device;
		nvmlPciInfo_t pci;
		nvmlComputeMode_t compute_mode;
		// Query for device handle to perform operations on a device
		// You can also query device handle by other features like:
		// nvmlDeviceGetHandleBySerial or nvmlDeviceGetHandleByPciBusId
		result = nvmlDeviceGetHandleByIndex(i, &device);
		if (NVML_SUCCESS != result) {
			printf("Failed to get handle for device %i: %s\n", i, nvmlErrorString(result));
			return 1;
		}
		result = nvmlDeviceGetName(device, nvml_info[i]->name, NVML_DEVICE_NAME_BUFFER_SIZE);
		if (NVML_SUCCESS != result) {
			printf("Failed to get name of device %i: %s\n", i, nvmlErrorString(result));
			return 1;
		}
		// pci.busId is very useful to know which device physically you're talking to
		// Using PCI identifier you can also match nvmlDevice handle to CUDA device.
		result = nvmlDeviceGetPciInfo(device, &pci);
		if (NVML_SUCCESS != result) {
			printf("Failed to get pci info for device %i: %s\n", i, nvmlErrorString(result));
			return 1;
		}else{
			allocate_and_asign(&nvml_info[i]->pcibusId, pci.busId, strlen(pci.busId)+1);
		}
		// This is a simple example on how you can modify GPU's state
		result = nvmlDeviceGetComputeMode(device, &compute_mode);
		if (NVML_ERROR_NOT_SUPPORTED == result)
			printf("\t This is not a CUDA capable device\n");
		else if (NVML_SUCCESS != result) {
			printf("Failed to get compute mode for device %i: %s\n", i, nvmlErrorString(result));
			return 1;
		} else {
			//************************ COLLECT METRIC temperature ************************
			result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &nvml_info[i]->temperature);
			if (NVML_SUCCESS == result){
				nvml_info[i]->valid_temperature=0;
			}else//register error message
				allocate_and_asign(&nvml_info[i]->temperatureErrorString, nvmlErrorString(result), (strlen(nvmlErrorString(result))+1) );
			//***************** COLLECT METRIC GPU-LOAD Usage-rate  ************************
			nvmlUtilization_t utilization;
			result = nvmlDeviceGetUtilizationRates(device, &utilization);
			if (NVML_SUCCESS == result){
				nvml_info[i]->valid_utilization=0;
				nvml_info[i]->utilizationgpu =utilization.gpu;
				nvml_info[i]->utilizationmemory =utilization.memory;
			}else
				allocate_and_asign(&nvml_info[i]->utilizationErrorString, nvmlErrorString(result), strlen(nvmlErrorString(result))+1);
			//************************ COLLECT METRIC  Power consumption************************ 
			result = nvmlDeviceGetPowerUsage (device, &nvml_info[i]->power);
			if (NVML_SUCCESS == result)
				nvml_info[i]->valid_power=0;
			else
				allocate_and_asign(&nvml_info[i]->powerErrorString, nvmlErrorString(result), strlen(nvmlErrorString(result))+1);
			//************************ COLLECT METRIC  FB memory ************************
			nvmlMemory_t memory;
			result = nvmlDeviceGetMemoryInfo(device, &memory);
			if (NVML_SUCCESS == result){
				nvml_info[i]->valid_memory=0;
				nvml_info[i]->memorytotal=memory.total;
				nvml_info[i]->memoryfree=memory.free;
				nvml_info[i]->memoryused=memory.used;
			}else
				allocate_and_asign(&nvml_info[i]->memoryErrorString, nvmlErrorString(result), strlen(nvmlErrorString(result))+1);
			//************************ COLLECT METRIC  BAR1 memory************************
			nvmlBAR1Memory_t bar1Memory;
			result = nvmlDeviceGetBAR1MemoryInfo(device, &bar1Memory);
			if (NVML_SUCCESS == result){
				nvml_info[i]->valid_bar1Memory=0;
				nvml_info[i]->bar1MemoryTotal=bar1Memory.bar1Total;
				nvml_info[i]->bar1MemoryFree=bar1Memory.bar1Free;
				nvml_info[i]->bar1MemoryUsed=bar1Memory.bar1Used;
			}else
				allocate_and_asign(&nvml_info[i]->bar1MemoryErrorString, nvmlErrorString(result), strlen(nvmlErrorString(result))+1);
			//************************ COLLECT METRIC clocks ************************
			//	NVML_CLOCK_GRAPHICS = 0 ; // Graphics clock domain.
			get_clock_info(device, 0, &nvml_info[i]->valid_Graphicsclock, &nvml_info[i]->Graphicsclock,
							&nvml_info[i]->Graphicsmax_clock,
							&nvml_info[i]->GraphicsclockErrorString);
			//	NVML_CLOCK_SM = 1 ; //SM clock domain.
			get_clock_info(device, 1, &nvml_info[i]->valid_SMclock, &nvml_info[i]->SMclock,
							&nvml_info[i]->SMmax_clock,
							&nvml_info[i]->SMclockErrorString);
			//	NVML_CLOCK_MEM = 2 ; //Memory clock domain.
			get_clock_info(device, 2, &nvml_info[i]->valid_Memoryclock, &nvml_info[i]->Memoryclock,
							&nvml_info[i]->Memorymax_clock,
							&nvml_info[i]->MemoryclockErrorString);
			//	NVML_CLOCK_VIDEO = 3 ; //Video encoder/decoder clock domain. It is not defined in the version of nvml in node01 !
			get_clock_info(device, 3, &nvml_info[i]->valid_Videoclock, &nvml_info[i]->Videoclock,
							&nvml_info[i]->Videomax_clock,
							&nvml_info[i]->VideoclockErrorString);
			//	NVML_CLOCK_COUNT
			//***************************** Information of PCIe utilization counters ***********************
#if(NVML_API_VERSION >= 7) // not defined in NVML_API_VERSION<=6 (320.29 2014), but defined in NVML_API_VERSION=7 (346.46 2015)
			unsigned int bytes_per_20ms;
			result = nvmlDeviceGetPcieThroughput(device, NVML_PCIE_UTIL_TX_BYTES, &bytes_per_20ms);
			/* Returns #bytes during a 20msec interval. Transform to bytes/sec */
			if (NVML_SUCCESS == result){
				nvml_info[i]->valid_pcie_tx=0;
				nvml_info[i]->pcie_tx=(float) bytes_per_20ms / 0.020;
			}else
				allocate_and_asign(&nvml_info[i]->pcie_tx_ErrorString, nvmlErrorString(result), strlen(nvmlErrorString(result))+1);
			result = nvmlDeviceGetPcieThroughput(device, NVML_PCIE_UTIL_RX_BYTES, &bytes_per_20ms);
			/* Returns #bytes during a 20msec interval. Transform to bytes/sec */
			if (NVML_SUCCESS == result){
				nvml_info[i]->valid_pcie_rx=0;
				nvml_info[i]->pcie_rx=(float) bytes_per_20ms / 0.020;
			}else
				allocate_and_asign(&nvml_info[i]->pcie_rx_ErrorString, nvmlErrorString(result), strlen(nvmlErrorString(result))+1);
#endif
			//***************************** Information about running compute processes on the GPU ***********************
			nvml_info[i]->ProcessCount=0;//amount of process
			result = nvmlDeviceGetComputeRunningProcesses(device, &nvml_info[i]->ProcessCount, infos);
			if (NVML_SUCCESS != result) {
				allocate_and_asign(&nvml_info[i]->ProcessCountErrorString, nvmlErrorString(result), strlen(nvmlErrorString(result))+1);
			}else{
				if (nvml_info[i]->metric_gpu_processes !=NULL){
					for (j = 0; j < nvml_info[i]->ProcessCount; j++)
						free(nvml_info[i]->metric_gpu_processes[j]);
					free(nvml_info[i]->metric_gpu_processes);
				}
				nvml_info[i]->valid_ProcessCount=0;
				//to be reserved dinamically depending on the amount of processes
				nvml_info[i]->metric_gpu_processes = (struct metric_gpu_processes**) malloc(1 * sizeof(struct metric_gpu_processes*));
				for (j = 0; j < nvml_info[i]->ProcessCount; j++)
					nvml_info[i]->metric_gpu_processes[j] = (struct metric_gpu_processes*) malloc(nvml_info[i]->ProcessCount * sizeof(struct metric_gpu_processes));
				for (j = 0; j < nvml_info[i]->ProcessCount; j++) {
					nvml_info[i]->metric_gpu_processes[j]->pid=infos[j].pid;
					nvml_info[i]->metric_gpu_processes[j]->usedGpuMemory=infos[j].usedGpuMemory;
				}
			}
		}
	}
	return 0;
}// end of nvml_get_stats

//return_code: 
// 2- Failed to initialize NVML
// 1- any other error
int nvml_monitor(int pid, char *DataPath, long sampling_interval){
	/*create and open the file*/
	char FileName[256] = {'\0'};
	sprintf(FileName, "%s/%s", DataPath, METRIC_NAME_4);
	FILE *fp = fopen(FileName, "a"); //append data to the end of the file
	if (fp == NULL) {
		printf("ERROR: Could not create file: %s\n", FileName);
		return 0;
	}
	struct timespec timestamp;
	double timestamp_ms;
/***** NVML metrics *******/
	/*initialize the values in result */
	nvmlReturn_t result;// it is expected to be an unsigned int betweem 0 and 999
	unsigned int device_count, i, j;
	unsigned int return_code=0;
	struct nvml_stats **nvml_info=NULL;
	// First initialize NVML library
	result = nvmlInit();
	if (NVML_SUCCESS != result) {
		printf("Failed to initialize NVML: %s\n There is an NVIDIA device and the Driver is runnning?\n", nvmlErrorString(result));
		device_count=0;
		printf("Press ENTER to continue...\n");
		getchar();
		return_code=2;
		goto End;
	}
	result = nvmlDeviceGetCount(&device_count);
	if (NVML_SUCCESS != result) {
		device_count=0;
		printf("Failed to query device count: %s\n", nvmlErrorString(result));
		return_code=1;
		goto End;
	}
	printf("Found %d device%s\n\n", device_count, device_count != 1 ? "s" : "");
	if(device_count==0){
		device_count=0;
		printf("closing nvml_monitoring because there were not NVIDIA devices found\n");
		return 0;
	}	
	
	
	nvml_info = (struct nvml_stats**) malloc(1 * sizeof(struct nvml_stats*));
	//Reservation of memory for one metric
	for (i = 0; i < device_count; i++) {
		nvml_info[i] = (struct nvml_stats*) malloc(device_count * sizeof(struct nvml_stats));
		nvml_info[i]->valid_temperature=-1; // equal 0 (NVML_SUCCESS) if successfuly measured
		nvml_info[i]->valid_utilization=-1;
		nvml_info[i]->valid_power=-1;
		nvml_info[i]->valid_memory=-1;
		nvml_info[i]->valid_bar1Memory=-1;
		nvml_info[i]->valid_Graphicsclock=-1;
		nvml_info[i]->valid_SMclock=-1;
		nvml_info[i]->valid_Memoryclock=-1;
		nvml_info[i]->valid_Videoclock=-1;
		nvml_info[i]->valid_ProcessCount=-1;
		nvml_info[i]->ProcessCount=0;
		nvml_info[i]->valid_pcie_tx=0;// only collect metrics if NVML_API_VERSION >= 7
		nvml_info[i]->valid_pcie_rx=0;// only collect metrics if NVML_API_VERSION >= 7
		nvml_info[i]->metric_gpu_processes=NULL;
		nvml_info[i]->pcibusId =NULL;
		nvml_info[i]->temperatureErrorString = NULL; //error message
		nvml_info[i]->utilizationErrorString = NULL;
		nvml_info[i]->powerErrorString = NULL;
		nvml_info[i]->memoryErrorString = NULL;
		nvml_info[i]->bar1MemoryErrorString = NULL;
		nvml_info[i]->ProcessCountErrorString =NULL;
		nvml_info[i]->GraphicsclockErrorString=NULL;
		nvml_info[i]->SMclockErrorString=NULL;
		nvml_info[i]->MemoryclockErrorString=NULL;
		nvml_info[i]->VideoclockErrorString=NULL;
		nvml_info[i]->pcie_tx_ErrorString=NULL;
		nvml_info[i]->pcie_rx_ErrorString=NULL;
	}
	char *metrics_json=NULL;
/***** end NVML metrics ****/
	if(nvml_get_stats(device_count, nvml_info)==1) goto End;
	/*in a loop do data sampling and write into the file*/
	while(running) {
		usleep(sampling_interval * 1000);
		/*get current timestamp in ms*/
		clock_gettime(CLOCK_REALTIME, &timestamp);
    	timestamp_ms = timestamp.tv_sec * 1000.0  + (double)(timestamp.tv_nsec / 1.0e6);
		/****** NVML ****/
			if(nvml_get_stats(device_count, nvml_info)==1) goto End;
			// ********************* REPORT METRICS ************************  
			for (i = 0; i < device_count; i++){
				metrics_json=NULL;
	// 			report_metrics(i, nvml_info[i]);
				get_json_metrics(i, nvml_info[i], &metrics_json);
				printf("%s\n",metrics_json);
				fprintf(fp, "\"local_timestamp\":\"%.1f\",\n%s,\n", timestamp_ms, metrics_json);
			}
		/*** NVML end ***/
	}
	//*******************************************************************************
	End:
	/*close the file*/
	fclose(fp);
	//*************************** FREE the reserved memory **************************
	for (i = 0; i < device_count; i++){
		if(nvml_info[i]->pcibusId !=NULL ) free(nvml_info[i]->pcibusId );
		if(nvml_info[i]->GraphicsclockErrorString!=NULL) free(nvml_info[i]->GraphicsclockErrorString);
		if(nvml_info[i]->SMclockErrorString!=NULL) free(nvml_info[i]->SMclockErrorString);
		if(nvml_info[i]->VideoclockErrorString!=NULL) free(nvml_info[i]->VideoclockErrorString);
		if(nvml_info[i]->MemoryclockErrorString!=NULL) free(nvml_info[i]->MemoryclockErrorString);
		if(nvml_info[i]->ProcessCountErrorString !=NULL) free(nvml_info[i]->ProcessCountErrorString);
		if(nvml_info[i]->valid_ProcessCount==0){
			for (j = 0; j < nvml_info[i]->ProcessCount; j++)
				free(nvml_info[i]->metric_gpu_processes[j]);
			free(nvml_info[i]->metric_gpu_processes);
		}
		free(nvml_info[i]);
	}
	if(nvml_info!=NULL) free(nvml_info);
	// Shutdown the NVML
	if(return_code!=2 ){
		result = nvmlShutdown();
		if (NVML_SUCCESS != result)
			printf("\nFailed to shutdown NVML: %s\n", nvmlErrorString(result));
	}
	return return_code;
}
