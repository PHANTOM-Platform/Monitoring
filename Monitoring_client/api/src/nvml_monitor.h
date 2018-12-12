#ifndef _NVML_MONITOR_H
#define _NVML_MONITOR_H

#include <nvml.h>

#define METRIC_NAME_4 "nvml"


struct metric_gpu_processes{//one register like this for each process
	unsigned int pid;
	unsigned long long int usedGpuMemory;//bytes
};

struct nvml_stats {
	char name[NVML_DEVICE_NAME_BUFFER_SIZE];
	char *pcibusId;
	int valid_temperature; // equal 0 (NVML_SUCCESS) if successfuly measured
		char *temperatureErrorString; // error message when valid_temperature, reserved dinamically based on needs
		unsigned int temperature;
	int valid_utilization;
		char *utilizationErrorString;
		unsigned long long int utilizationgpu ;
		unsigned long long int utilizationmemory;
	int valid_power;
		char *powerErrorString;
		unsigned int power; //mw nvmlReturn_t
	int valid_memory;
		char *memoryErrorString;
		unsigned long long int memorytotal; //bytes
		unsigned long long int memoryfree; //bytes
		unsigned long long int memoryused; //bytes
	int valid_bar1Memory;
		char *bar1MemoryErrorString;
		unsigned long long int bar1MemoryTotal;//bytes
		unsigned long long int bar1MemoryFree; //bytes
		unsigned long long int bar1MemoryUsed; //bytes

	int valid_pcie_tx; // not defined in NVML_API_VERSION<=6 (320.29 2014), but defined in NVML_API_VERSION=7 (346.46 2015)
	//http://developer.download.nvidia.com/compute/cuda/7_0/Prod/local_installers/cuda_346.46_gdk_linux.run
		char *pcie_tx_ErrorString;
		float pcie_tx; // 1KB granularity
	int valid_pcie_rx; // not defined in NVML_API_VERSION<=6, but defined in NVML_API_VERSION=7
		char *pcie_rx_ErrorString;
		float pcie_rx; // 1KB granularity

	int valid_Graphicsclock;
		char *GraphicsclockErrorString;  // Graphics clock domain.
		unsigned int Graphicsclock;// MHz
		unsigned int Graphicsmax_clock;
	int valid_SMclock;
		char *SMclockErrorString; //SM clock domain.
		unsigned int SMclock; // MHz
		unsigned int SMmax_clock;
	int valid_Memoryclock;
		char *MemoryclockErrorString; //Memory clock domain.
		unsigned int Memoryclock; // MHz
		unsigned int Memorymax_clock; 
	int valid_Videoclock;
		char *VideoclockErrorString;  //Video encoder/decoder clock domain.
		unsigned int Videoclock; // MHz
		unsigned int Videomax_clock;
	// 	NVML_CLOCK_COUNT
	int valid_ProcessCount;
		char *ProcessCountErrorString;
		unsigned int ProcessCount; //amount of process
		struct metric_gpu_processes **metric_gpu_processes; //to be reserved dinamically depending on the amount of processes
};

// const char* convertToComputeModeString(const nvmlComputeMode_t mode) {
// 	switch (mode) {
// 		case NVML_COMPUTEMODE_DEFAULT:
// 			return "Default";
// 		case NVML_COMPUTEMODE_EXCLUSIVE_THREAD:
// 			return "Exclusive_Thread";
// 		case NVML_COMPUTEMODE_PROHIBITED:
// 			return "Prohibited";
// 		case NVML_COMPUTEMODE_EXCLUSIVE_PROCESS:
// 			return "Exclusive Process";
// 		default:
// 			return "Unknown";
// 	}
// }

int nvml_get_stats(const int device_count, struct nvml_stats **nvml_info);
int nvml_monitor(int pid, char *DataPath, long sampling_interval);

#endif
