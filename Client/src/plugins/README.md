# Introduction of plugins and usage information

## Introduction
The monitoring client is composed of 7 plugins, monitoring all kinds of the system infrastructure-level performance and power metrics. The plugins implemented are based on various libraries, system hardware counters and Linux proc filesystem. In addition to be used by the monitoring client, each plugin can be built as a standalone client, being executed alone with given specific metrics name. 

More details about each plugin, for example, the plugins' usage, prerequisites and supported metrics are all clarified in the following.

### List of plugins

- Board_power
- CPU_perf
- CPU_temperature
- Linux_resources
- Linux_sys_power
- NVML
- RAPL_power

## Board_power Plugin

This plugin is based on the external ACME power measurement kit and the libiio library, which is installed during the monitoring client setup process, done automatically by the setup.sh shell script. In case that the ACME power measurement board is not connected with the monitoring client hosted computer or the libiio library is not found, the plugin will fail and report associated error messages.

The default hostname of the ACME board is **"baylibre-acme.local"** or **"power-jetson.local"** depending on the images you use on the ACME board.

It is recommended to try the following command to test the ACME power measurement board connection:

```
$ ping baylibre-acme.local
```
or 

```
$ ping power-jetson.local
```

Please configure the ACME board name in the **mf_config.ini** file.


### Usage and metrics

The Board_power plugin can be built and ran alone, outside the monitoring framework. In the directory of the plugin, execute the **Makefile** using

```
$ make clean all
```

, before calling the generated sampling client **mf_Board_power_client**, please add the required libraries to the **LD_LIBRARY_PATH** by:

```
$ source setenv.sh
```

It is advised to run the sampling client **mf_Board_power_client** like the follows:

```
$ ./mf_Board_power_client <LIST_OF_Board_power_METRICS>
```

Replace **<LIST_OF_Board_power_METRICS>** with a space-separated list of the following events:

- device0:current
- device0:vshunt
- device0:vbus
- device0:power

Unit and description for each metric is showed in the following table:

| Metrics         | Units  | Description   |
|---------------- |------  |-------------  |
| device0:current | mA     | Current measured for the target board |
| device0:vshunt  | mV     | Shunt voltage drop |
| device0:vbus    | mV     | Bus supply voltage |
| device0:power   | mW     | Power measured for the target board |


## CPU_perf Plugin

This plugin is based on the PAPI library and associated system hardware counters (PAPI_FP_INS, PAPI_FP_OPS, PAPI_TOT_INS). The PAPI library is installed during the monitoring client setup process, done by the setup.sh shell script. In case that the PAPI library is not found or the associated PAPI counters are not available, the plugin will fail at the initialization stage.

### Usage and metrics

The CPU_perf plugin can be built and ran alone, outside the monitoring framework. In the directory of the plugin, execute the **Makefile** using

```
$ make all
```

, before calling the generated sampling client **mf_CPU_perf_client**, please add the required libraries to the **LD_LIBRARY_PATH** by:

```
$ source setenv.sh
```

It is advised to run the sampling client **mf_CPU_perf_client** with root permissions, like:

```
$ ./mf_CPU_perf_client <LIST_OF_CPU_perf_METRICS>
```

Replace **<LIST_OF_CPU_perf_METRICS>** with a space-separated list of the following events:

- MFLIPS
- MFLOPS
- MIPS

It is noted that the default number of cores for the application mf_CPU_perf_client is 10. Unit and description for each metric is showed in the following table:

| Metrics    | Units    | Description   |
|----------- |--------  |-------------  |
| MFLIPS     | Mflip/s  | Mega floating-point instructions per second |
| MFLOPS     | Mflop/s  | Mega floating-point operations per second |
| MIPS       | Mip/s    | Million instructions per second |


## CPU_temperature Plugin

This plugin is based on the lm_sensors library which provides tools to and drivers to monitor CPU temperatures and other properties. The lm_sensors library is installed during the monitoring client setup process, done by the setup.sh shell script. In case that the lm_sensors library is not found, the plugin will fail at the initialization stage.

### Usage and metrics

The CPU_temperature plugin can be built and ran alone, outside the monitoring framework. In the directory of the plugin, execute the **Makefile** using

```
$ make all
```

, before calling the generated sampling client **mf_CPU_temperature_client**, please add the required libraries to the **LD_LIBRARY_PATH** by:

```
$ source setenv.sh
```

It is advised to run the sampling client **mf_CPU_temperature_client** like the follows:

```
$ ./mf_CPU_temperature_client <LIST_OF_CPU_temperature_METRICS>
```

Replace **<LIST_OF_CPU_temperature_METRICS>** with a space-separated list of the following events:

- CPU[i]:core[ii]

, where i represents the CPU socket id, ii identifies which core is the target core of monitoring. The units of all CPU_temperature metrics are degree (°c).


## Linux_resources Plugin

This plugin is based on the Linux proc filesystem which provides information and statistics about processes and system.

### Usage and metrics

The Linux_resources plugin can be built and ran alone, outside the monitoring framework. In the directory of the plugin, execute the **Makefile** using

```
$ make all
```

will build the standalone executable client **mf_Linux_resources_client**. It is advised to run the sampling client like the follows:

```
$ ./mf_Linux_resources_client <LIST_OF_Linux_resources_METRICS>
```

Replace **<LIST_OF_Linux_resources_METRICS>** with a space-separated list of the following events:

- CPU_usage_rate
- RAM_usage_rate
- swap_usage_rate
- net_throughput
- io_throughput

Unit and description for each metric is showed in the following table:

| Metrics         | Units    | Description                                                   |
|---------------- |--------- |-------------------------------------------------------------  |
| CPU_usage_rate  | %        | Percentage of CPU usage time                                  |
| RAM_usage_rate  | %        | Percentage of used RAM size                                   |
| swap_usage_rate | %        | Percentage of used swap size                                  |
| net_throughput  | bytes/s  | Total send and receive bytes via wlan and ethernet per second |
| io_throughput   | bytes/s  | Total disk read and write bytes per second                    |


## Linux_sys_power Plugin

This plugin is based on the Linux proc filesystem and some system drivers and libraries. The CPU power consumption is estimated by using a Linux module named as [cpufreq-stats][cpufreq-stats-module]. It is a driver that provides CPU frequency statistics for each CPU through its interface, which appears normally in the directory `/sysfs/devices/system/cpu/cpuX/cpufreq/stats`. For memory power estimation, we uses the system call [“__NR_perf_event_open”][perf-event-open] to stat the hardware cache misses . Together with reading the disk I/O read/write statistics, we calculate the memory power consumption with the following formula. The L2 cache miss latency and L2 cache line size can be obtained via some known [calibrator][caliborator]. Disk and wireless network power consumptions are calculated based on their activities, like read/write and receive/send bytes during the sampling interval. As long as the energy specifications of the disk and wireless network card are given, we could compute the constants, like energy cost per disk read/write and energy cost per wireless network receive/send (`E_DISK_R_PER_KB`, `E_DISK_W_PER_KB`, `E_NET_SND_PER_KB`, `E_NET_RCV_PER_KB`), and get finally the energy consumed during a specific period.

Please refer to the [pTop][ptop] project for more design and methodology details.


### Usage and metrics

The Linux_sys_power plugin can be built and ran alone, outside the monitoring framework. In the directory of the plugin, execute the **Makefile** using

```
$ make all
```

It is advised to run the sampling client **mf_Linux_sys_power_client** with root permissions, like:

```
$ ./mf_Linux_sys_power_client <LIST_OF_Linux_sys_power_METRICS>
```

Replace **<LIST_OF_Linux_sys_power_METRICS>** with a space-separated list of the following events:

- estimated_CPU_power
- estimated_memory_power
- estimated_wifi_power
- estimated_disk_power
- estimated_total_power

Unit and description for each metric is showed in the following table:

| Metrics                | Units      | Description                                                          |
|----------------------- |----------  |--------------------------------------------------------------------  |
| estimated_CPU_power    | milliwatts | Estimated CPU power consumption                                      |
| estimated_memory_power | milliwatts | Main memory power, which is estimated using the system calls         |
| estimated_wifi_power   | milliwatts | Estimated wireless network power                                     |
| estimated_disk_power   | milliwatts | Estimated disk power                                                 |
| estimated_total_power  | milliwatts | Total system power, calculated by the addition of the abover metrics |


## NVML Plugin

This plugin is based on the nvml ([NVIDIA management library][nvml]), which is a C-based API for monitoring various states of the NVIDIA GPU devices. It is required to use the library **libnvidia-ml.so**, which is installed with the NVIDIA Display Driver, therefore the default library path (`/usr/lib64` or `/usr/lib`) is used to link and build the plugin. During the monitoring setup process, done the setup.sh shell script, the NVIDIA SDK with appropriate header file is installed.

### Usage and metrics

The NVML plugin can be built and ran alone, outside the monitoring framework. In the directory of the plugin, execute the **Makefile** using
```
$ make all
```

will build the standalone executable client **mf_NVML_client**. Before calling the generated sampling client **mf_CPU_temperature_client**, please add the required libraries to the **LD_LIBRARY_PATH** by:
```
$ source setenv.sh
```

It is advised to run the sampling client with root permissions like the follows:
```
$ ./mf_NVML_client <LIST_OF_NVML_METRICS>
```

Replace **<LIST_OF_NVML_METRICS>** with a space-separated list of the following events:

- gpu_usage_rate
- mem_usage_rate
- mem_allocated
- PCIe_snd_throughput
- PCIe_rcv_throughput
- temperature
- power


All the metrics above is reported per GPU device. Unit and description for each metric is showed in the following table:

| Metrics             | Units      | Description   |
|-------------------- |----------- |-------------  |
| gpu_usage_rate      | %          | Percentage of time for GPU execution |
| mem_usage_rate      | %          | Percent of time for device memory access |
| mem_allocated       | %          | Percentage of used device memory |
| PCIe_snd_throughput | bytes/s    | PCIe send bytes per second (for Maxwell or newer architecture) |
| PCIe_rcv_throughput | bytes/s    | PCIe write bytes per second (for Maxwell or newer architecture) |
| temperature         | °c         | GPU device temperature |
| power               | milliwatts | GPU device power consumption |


## RAPL_power Plugin

This plugin is implemented for Intel CPU power measurements, as using [RAPL][rapl] provided energy and power information. In general cases, RAPL reports energy measurements per CPU socket covering basically two domains: the CPU package (including core and uncore devices) and the DRAM. 

### Usage and metrics

The RAPL_power plugin can be built and ran alone, outside the monitoring framework. In the directory of the plugin, execute the **Makefile** using 
```
$ make all
```

will build the standalone executable client **mf_RAPL_power_client**. Before calling the generated sampling client, please add the required libraries to the **LD_LIBRARY_PATH** by:
```
$ source setenv.sh
```

It is advised to run the sampling client with root permissions like the following:
```
$ ./mf_RAPL_power_client <LIST_OF_RAPL_POWER_METRICS>
```

Replace **<LIST_OF_RAPL_POWER_METRICS>** with a space-separated list of the following events:
- total_power
- dram_power

All the metrics above is reported per CPU socket. Unit and description for each metric is showed in the following table:

| Metrics             | Units      | Description   |
|-------------------- |----------- |-------------  |
| total_power         | milliwatts | Total CPU package power |
| dram_power          | milliwatts | Total DRAM power |


[cpufreq-stats-module]: https://www.kernel.org/doc/Documentation/cpu-freq/cpufreq-stats.txt
[perf-event-open]: http://man7.org/linux/man-pages/man2/perf_event_open.2.html
[caliborator]: http://homepages.cwi.nl/~manegold/Calibrator/calibrator.shtml
[ptop]: http://mist.cs.wayne.edu/ptop.html
[nvml]: https://developer.nvidia.com/nvidia-management-library-nvml
[rapl]: https://01.org/zh/blogs/2014/running-average-power-limit-%E2%80%93-rapl?langredirect=1
