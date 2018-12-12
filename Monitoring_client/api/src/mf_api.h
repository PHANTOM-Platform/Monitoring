#ifndef _MF_API_H
#define _MF_API_H

#define MAX_NUM_METRICS     9 
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

/**
* Get the pid, and setup the DataPath for data storage 
* For each metric, create a thread, open a file for data storage, and start sampling the metrics periodically.
* Return the path of data files
*/
char *mf_start(const char *server,const char *platform_id, metrics *m,const char *token);

/**
* Stop threads.
* Close all the files for data storage.
*/
void mf_end(void);

/**
* Query for a workflow, return 400 if the workflow is not registered yet.
* or 200 in other case.
*/
char* mf_query_workflow(char *server, char *application_id );

/**
* Register a new workflow.
* @return the path to query the workflow.
*/
char* mf_new_workflow(char *server, char *application_id, char *author_id,
		char *optimization, char *tasks_desc, char *token);

/**
* Generate the execution_id.
* Send the monitoring data in all the files to mf_server.
* @return the execution_id
*/
char *mf_send(const char *server,const  char *application_id,const  char *component_id,const  char *platform_id,const  char *token);

/* additonal functions developed during the integration */
struct Thread_report {
	char taskid[50];
	long long int start_time;
	long long int end_time;
	unsigned int total_metrics;
	char **user_label;
	char **user_value;
	char **metric_time;	
};

typedef struct metric_query_t {
	char *query ;
	int multiple_fields;
} metric_query;

metric_query *new_metric(const char* label);

metric_query *add_int_field(metric_query* user_query, const char* label, const unsigned int total, int array_int[] ) ;

metric_query *add_string_field(metric_query* user_query, const char* label, const unsigned int total, char **array_str );
void submit_metric(metric_query *user_query); 

/**
* @return the current time in us
* requires: #include <sys/time.h> */
long long int mycurrenttime (void);

char *mycurrenttime_str (void) ;
void start_monitoring(char *server, char *regplatformid);

void user_metrics_buffer(char *currentid, struct Thread_report single_thread_report );

void register_end_component( 
char *currentid, struct Thread_report single_thread_report );

void monitoring_send(char *server,char *appid, char *execfile, char *regplatformid, char *token);
	
int register_workflow( char *server, char *regplatformid, char *appid, char *execfile, char *token);
#endif /* _MF_API_H */
