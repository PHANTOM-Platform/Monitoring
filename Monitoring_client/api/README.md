# PHANTOM monitoring client APIs

> PHANTOM monitoring client APIs are used in mainly two domains: application-level monitoring for code instrumentation; and user-defined metrics sampling and sending. Although the APIs are written in C, it can be used also by applications written in other languages as well. In the following, we will clarify how and when to use these APIs in an application.

## Interfaces introduction
Following interfaces are developed and included in the first release of the monitoring client:

```
char *mf_start(char *server, char *platform_id, metrics *m);

void mf_end(void);

char *mf_send(char *server, char *application_id, char *component_id, char *platform_id);

int mf_user_metric(char *metric_name, char *value);
```

Function **mf_start** starts monitoring of the predefined metrics for sub-components of an application. Data are stored at first locally. Required input parameters should include the MF server URL, name of the platform (where the application runs), and the metrics’ name and sampling frequency. 

Function **mf_stop** stops monitoring of the predefined metrics when the sub-component is finished.

Function **mf_send** sends locally-stored predefined metrics to the PHANTOM MF server. The unique generated execution ID will be returned on success. 

Function **mf_user_metric** sends user-defined metrics with given metric’s name, value, and current local timestamps to the PHANTOM MF server. It is noted that the programmers should convert the metrics' value into a string while calling this function.

## Application example
The above mentioned APIs can be used by a generic application for code instrumentation. Following is a part of an application's source code (written in C++), which is used here to clarify how to use the client APIs. 

As specified by the programmer, if all preconditions are statisfied by the target platform, we can gather power and performance metrics about CPU, memory, disk I/O and network devices, as defined in the variable **m_resources**. In addition to these predefined metrics, users are able to send also user-defined metrics to the MF server. Take the following source code as an example, the user-defined metrics are **duration** and **nrLoops**, which represent the execution duration for each simulation and the number of total simulation loops respectively.

```
#include ...
extern "C" {
	#include "../src/mf_api.h"
}

...
int main() {
/* initialization of various application elements, and parameters */
	...
	initNetwork("TestNet", &netparams, 8, 6);
	initBranches(&(*branches), netparams);
	initVertexes(&(*vertexes), netparams);
	float integrationStep = 0.001; 	// simulation steps
	int nrLoops = 5000; 			// simulation loops

/* MONITORING START */
	metrics m_resources;
	m_resources.num_metrics = 3;
	m_resources.local_data_storage = 1;
	m_resources.sampling_interval[0] = 1000; // 1000 ms
	strcpy(m_resources.metrics_names[0], "resources_usage");
	m_resources.sampling_interval[1] = 1000; // 1000 ms
	strcpy(m_resources.metrics_names[1], "disk_io");
	m_resources.sampling_interval[2] = 1000; // 1000 ms
	strcpy(m_resources.metrics_names[2], "power");
	char *datapath = mf_start("141.58.0.8:3033", "node01", &m_resources);

/* simulation process */
	for (int n = 0; n < nrLoops; n++) {
		auto begin_time = std::chrono::high_resolution_clock::now();
		simulation_loop(&(*branches), &(*vertexes), netparams, n,
				integrationStep);
		auto end_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> duration = end_time-begin_time;

	/* MONITORING USER-DEFINED METRICS → duration of each loop */
		char metric_value[8] = {'\0'};
		sprintf(metric_value, "%f", duration);
		mf_user_metric("duration", metric_value);
	}

/* MONITORING END */
	mf_end();

/* MONITORING USER-DEFINED METRICS →total nr. of completed loops */
	char metric_value[8] = {'\0'};
	sprintf(metric_value, "%d", nrLoops);
	mf_user_metric("nrLoops", metric_value);	

/* MONITORING SEND */
	char *experiment_id = mf_send("141.58.0.8:3033", "dummy", "t1", "node01");
	printf("\n> experiment_id is %s\n", experiment_id);
	cout << "Simulation finished";
	return 0;
}
```