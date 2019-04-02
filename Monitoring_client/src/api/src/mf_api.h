#ifndef _MF_API_H
#define _MF_API_H


#define MAX_NUM_METRICS     9
#define NAME_LENGTH          32

#define SUCCESS 0
#define FAILED 1

#define max_report_tids 10

typedef struct task_data_t {
// 	unsigned int tids[max_report_tids];
	unsigned long int microsleep;
	int pid;
	int maxprocesses, maxcores;
	unsigned int maxtotaltid;
	unsigned int totaltid;//counts the total of subtasks
	struct sub_task_data **subtask;  //for the subtasks running
	unsigned int total_user_def; //counts the total of user_def params
	struct sub_task_user_def **task_def;  //for the of user_def params
	struct cores_data *cores;  //for the total of cores running
	float total_load_cpu, totalpmem;
	long long int first_start,last_end;
}task_data;

typedef struct metrics_t {
	long sampling_interval[MAX_NUM_METRICS];	//in milliseconds
	char metrics_names[MAX_NUM_METRICS][NAME_LENGTH];	//user defined metrics
	int num_metrics;
	int local_data_storage;
} metrics;

extern int running;
extern int keep_local_data_flag;
extern char parameters_name[9][32];
// extern float parameters_value[9];

/* additonal functions developed during the integration */
typedef struct Thread_report_t {
	char taskid[50];
	unsigned long int workerId;
// 	char *component_name;
	long long int start_time;
	long long int end_time;
	unsigned int total_metrics;
	double start_timestamp_ms;
	char currentid[100];
// 	double end_timestamp_ms;
	char **user_label;
	char **user_value;
	char **metric_time;
} Thread_report;

typedef struct app_report_t{
	long long int start_app;

	struct Thread_report_t **my_thread_report;
	unsigned int num_of_threads;
	unsigned int num_of_processes;
	double timestamp_ms;
	char currentid[100];
	float total_cpu_energy;
	float pid_mem_power, pid_disk_power;// pid_cpu_power,duration, sys_cpu_power,
	long long int pid_l2_cache_misses;
	long long int read_bytes;
	long long int write_bytes;
	long long int cancelled_writes;
	float pid_net_power;
	float total_hd_energy;
	float total_watts;
}app_report;

typedef struct each_metric_t {
	struct task_data_t *my_task_data_a;
	long sampling_interval;				//in milliseconds
	char metric_name[NAME_LENGTH];		//user defined metrics
	long long int start_app_time;
	struct app_report_t *my_app_report;
} each_metric;

typedef struct metric_query_t {
	char *query;
	int multiple_fields;
} metric_query;



struct app_report_t *reserve_app_report(const unsigned int num_of_threads,const char *currentid);
int free_app_report(struct app_report_t *my_app_report);
// int reserve_memory_user_def_metrics(struct Thread_report_t *my_thread_report, const unsigned int total_metrics);
int register_user_metric(struct Thread_report_t *my_thread_report,const char *time,const char *label,const int value,const int num_metric);

metric_query *new_metric(const char* label);

metric_query *add_int_field(metric_query* user_query, const char* label, const unsigned int total, int array_int[]);

metric_query *add_string_field(metric_query* user_query, const char* label, const unsigned int total, char **array_str);
void submit_metric(metric_query *user_query);

/**
* @return the current time in us
* requires: #include <sys/time.h> */
long long int mycurrenttime (void);

char *mycurrenttime_str (void);
void start_monitoring(char *server, char *regplatformid);

void user_metrics_buffer(struct Thread_report_t single_thread_report);

void register_end_component(
char *currentid, struct Thread_report_t single_thread_report);

void monitoring_end(const char *server, const char *exec_server, const char *appid,const char *execid, const char *execfile, const char *regplatformid, const char *token, struct app_report_t *my_app_report);
	
int register_workflow(const char *server, const char *regplatformid, const char *appid, const char *execfile,const char *token);

//for monitoring the threads
int mf_start_tid(const unsigned int pid, const long unsigned int microsleep);
void add_tid_to_report(char *component_name, int tid);

int mf_user_metric(char *metric_name, char *value);

/**
* Get the pid, and setup the DataPath for data storage 
* For each metric, create a thread, open a file for data storage, and start sampling the metrics periodically.
* Return the path of data files
*/
char *mf_start(const char *server, const char *exec_server, const char *exec_id, const char* resource_manager, const char *platform_id, metrics *m,struct app_report_t *my_app_report, const char *token);

/**
* Stop threads.
* Close all the files for data storage.
*/
void mf_end(void);

/**
* Query for a workflow, return 400 if the workflow is not registered yet.
* or 200 in other case.
*/
char* mf_query_workflow(const char *server, const char *application_id );

/**
* Register a new workflow.
* @return the path to query the workflow.
*/
char* mf_new_workflow(const char *server, const char *application_id, const char *author_id,
		const char *optimization, const char *tasks_desc, const char *token);

/**
* Generate the execution_id.
* Send the monitoring data in all the files to mf_server.
* @return the execution_id
*/
char* mf_send(const char *server,const  char *application_id,const  char *component_id,const  char *platform_id,const  char *token);

#endif /* _MF_API_H */
