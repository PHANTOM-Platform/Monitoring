
struct Thread_report {
	char  taskid[50];
	long long int start_time;
	long long int end_time;
	unsigned int number_of_blocks;
	int n_ships_found; 
};
 
//returns the current time in us
//requires: #include <sys/time.h>
long long unsigned int mycurrenttime (void) ;

long long int start_monitoring(char *server, char *regplatformid);

void prepare_user_metrics(  char *currentid, char *DataPath, int *pid, struct Thread_report single_thread_report );

void stop_monitoring();
 
void monitoring_send();
