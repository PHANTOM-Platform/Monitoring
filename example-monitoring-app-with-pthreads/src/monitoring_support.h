#ifndef MONITORING_SUPPORT_H
#define MONITORING_SUPPORT_H

struct Thread_report {
	char  taskid[50];
	long long unsigned int start_time;
	long long unsigned int end_time;
	unsigned int number_of_blocks;
	int n_ships_found; 
};
// long long int start_monitoring(char *server, char *regplatformid);
// void prepare_user_metrics(  char *currentid, char *DataPath, int *pid, struct Thread_report single_thread_report );
// void stop_monitoring();
// void monitoring_send();

typedef struct metric_query_t {
	char *query ;
	int multiple_fields;
} metric_query;

//Function for increase dynamically a string concatenating strings at the end
//It free the memory of the first pointer if not null
char* concat_and_free(char **s1, const char *s2);

char* itoa(int i, char b[]);

metric_query *new_metric(const char* label);

metric_query *add_int_field(metric_query* user_query, const char* label, const unsigned int total, int array_int[] ) ;

metric_query *add_string_field(metric_query* user_query, const char* label, const unsigned int total, char **array_str );
void submit_metric(metric_query *user_query); 

//returns the current time in us
//requires: #include <sys/time.h>
long long int mycurrenttime (void);

char *mycurrenttime_str (void) ;
void start_monitoring(const char *server, const char *regplatformid);

void prepare_user_metrics(char *currentid,struct Thread_report single_thread_report ) ;

void stop_monitoring(const char *server, const char *appid,  const char *execfile, const char *regplatformid);

void register_workflow( const char *server, const char *regplatformid, const char *appid, const char *execfile);
#endif
