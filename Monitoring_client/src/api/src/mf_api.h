#ifndef _MF_API_H
#define _MF_API_H

#define MAX_NUM_METRICS      3
#define NAME_LENGTH          32

typedef struct metrics_t {
	long sampling_interval[MAX_NUM_METRICS];	//in milliseconds
	char metrics_names[MAX_NUM_METRICS][NAME_LENGTH];	//user defined metrics
	int num_metrics;
	int local_data_storage;
} metrics;

extern int running;
extern int keep_local_data_flag;
extern char parameters_name[9][32];
extern float parameters_value[9];

int mf_user_metric(char *metric_name, char *value);
/* 
Get the pid, and setup the DataPath for data storage 
For each metric, create a thread, open a file for data storage, and start sampling the metrics periodically.
Return the path of data files
*/
char *mf_start(const char *server, const char *platform_id, metrics *m);

/*
Stop threads.
Close all the files for data storage
*/
void mf_end(void);

/*
Query for a workflow, return 400 if the workflow is not registered yet.
or 200 in other case.
*/
char* mf_query_workflow(const char *server, const char *application_id );
/*
Resgister a new workflow.
Return the path to query the workflow.
*/
char* mf_new_workflow(const char *server, const char *application_id, const char *author_id,
		const char *optimization, const char *tasks_desc);

/*
Generate the execution_id.
Send the monitoring data in all the files to mf_server.
Return the execution_id
*/
char *mf_send(const char *server, const char *application_id, const char *component_id, const char *platform_id);
#endif /* _MF_API_H */
