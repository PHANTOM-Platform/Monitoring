#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
//#include "resources_monitor.h"
//#include "disk_monitor.h"
//#include "power_monitor.h"
#include "mf_api.h"

/* some dummy application */
void dummy(void)
{
	int i, j;
	float x1, x2, result[1024*1024];
	char *tmp = calloc(1024, sizeof(char));

	for (i = 0; i < 1024 * 1024; i++) {
		x1 = (float)rand()/(float)(RAND_MAX/100.0);
		x2 = (float)rand()/(float)(RAND_MAX/1.0);
		result[i] = x1 * x2;
	}

	FILE *fp = fopen("./result.dat", "w");
	for (i = 0; i < 1024 * 1024; i++) {
		memset(tmp, '\0', 1024);
		sprintf(tmp, "result[%d] = %.3f", i, result[i]);
		for (j = 0; j < 16; j++) {
			strcat(tmp, "* ");
		}
		fprintf(fp, "* * %s\n", tmp);
	}

	fclose(fp);
}

/*******************************************************************************
 * resources_monitor test
 ******************************************************************************/
/* test resources_stat_cpu */
/*
void Test_resources_stat_cpu(void)
{
	int pid = getpid();
	resources_cpu cpu_stats_now;
	resources_stat_cpu(pid, &cpu_stats_now);
	printf("process %d\n\tprocess_CPU_time ---%llu\n\tglobal_CPU_time ---%llu\n", 
		pid, 
		cpu_stats_now.process_CPU_time, 
		cpu_stats_now.global_CPU_time);
	//wait for check
	char c;
	scanf("%c", &c);
}*/

/* test resources_stat_all_and_calculate */
/*
void Test_resources_stat_all_and_calculate(void)
{
	char c;
	int pid = getpid();
	resources_stats cpu_stats;
	resources_cpu cpu_stats_before, cpu_stats_after;

	//stats the current cpu time of the process and of the whole system
	resources_stat_cpu(pid, &cpu_stats_before);
	printf("process %d\n\tprocess_CPU_time ---%llu\n\tglobal_CPU_time ---%llu\n", 
		pid, 
		cpu_stats_before.process_CPU_time, 
		cpu_stats_before.global_CPU_time);
	//wait for check
	scanf("%c", &c);
	
	//do some dummy calculation 
	dummy();

	//stats the current cpu time of the process and of the whole system
	resources_stat_cpu(pid, &cpu_stats_after);
	printf("process %d\n\tprocess_CPU_time ---%llu\n\tglobal_CPU_time ---%llu\n", 
		pid, 
		cpu_stats_after.process_CPU_time, 
		cpu_stats_after.global_CPU_time);
	//wait for check
	scanf("%c", &c);

	//do stats of ram and swap and calculate the cpu_usage_rate for the duration
	resources_stat_all_and_calculate(pid, &cpu_stats_before, &cpu_stats_after, &cpu_stats);
	printf("process %d\n\tCPU_usage_rate ---%.3f\n\tRAM_usage_rate ---%.3f\n\tswap_usage_rate ---%.3f\n",
		pid,
		cpu_stats.CPU_usage_rate,
		cpu_stats.RAM_usage_rate, 
		cpu_stats.swap_usage_rate);
	//wait for check
	scanf("%c", &c);	
}
*/

/* test resources_monitor */
void Test_resources_monitor(void)
{
	int i;
	metrics m_resources;
	m_resources.num_metrics = 1;
	m_resources.local_data_storage = 1;
	m_resources.sampling_interval[0] = 1000; // 1s
	strcpy(m_resources.metrics_names[0], "resources_usage");

	char server[] = "localhost:3040";
	char platform_id[] = "fangli_laptop";

	char *datapath = mf_start(server, platform_id, &m_resources);
	printf("datapath is :%s\n", datapath);
	sleep(5);
	
	/*do dummy things*/
	for(i = 0; i < 10; i++) {
		dummy();
		sleep(2);
	}

	mf_end();
}

/*******************************************************************************
 * disk_monitor test
 ******************************************************************************/
/* test disk_stats_read */
/*void Test_disk_stats_read(void)
{
	char c;
	int pid = getpid();
	disk_stats disk_info;

	disk_info.read_bytes_after = 0;
	disk_info.write_bytes_after = 0;

	disk_stats_read(pid, &disk_info);

	printf("process %d\n\tread_bytes_before ---%llu\n\twrite_bytes_before ---%llu\n", 
		pid, 
		disk_info.read_bytes_before,
		disk_info.write_bytes_before);
	//wait for check
	scanf("%c", &c);

	dummy();

	disk_stats_read(pid, &disk_info);

	printf("process %d\n\tread_bytes_after ---%llu\n\twrite_bytes_after ---%llu\n", 
		pid, 
		disk_info.read_bytes_after,
		disk_info.write_bytes_after);
	//wait for check
	scanf("%c", &c);
}*/

/* test disk_monitor */
void Test_disk_monitor(void)
{
	int i;
	metrics m_resources;
	m_resources.num_metrics = 1;
	m_resources.local_data_storage = 1;
	m_resources.sampling_interval[0] = 1000; // 1s
	strcpy(m_resources.metrics_names[0], "disk_io");
	
	char server[] = "localhost:3040";
	char platform_id[] = "fangli_laptop";

	char *datapath = mf_start(server, platform_id, &m_resources);
	printf("datapath is :%s\n", datapath);
	sleep(5);
	
	//do dummy things
	for(i = 0; i < 10; i++) {
		dummy();
		sleep(2);
	}

	mf_end();
}
/*******************************************************************************
 * power_monitor test
 ******************************************************************************/
/*
void Test_CPU_power_read(void)
{
	char c;
	int pid = getpid();

	pid_stats_info before, after;
	
	memset(&before, 0, sizeof(pid_stats_info));
	memset(&after, 0, sizeof(pid_stats_info));

	if(read_pid_time(pid, &before) <= 0)
		printf("ERROR: read_pid_time failed!\n");

	if(read_sys_time(&before) <= 0)
		printf("ERROR: read_sys_time failed!\n");

	if(cpu_freq_stat(&before) <= 0)
		printf("ERROR: cpu_freq_stat failed!\n");

	dummy();

	if(read_pid_time(pid, &after) <= 0)
		printf("ERROR: read_pid_time failed!\n");

	if(read_sys_time(&after) <= 0)
		printf("ERROR: read_sys_time failed!\n");

	if(cpu_freq_stat(&after) <= 0)
		printf("ERROR: cpu_freq_stat failed!\n");

	printf("values read for process %d are:\n", pid);
	printf("system total time: %llu\n", (after.sys_itv - before.sys_itv));
	printf("system runtime:    %llu\n", (after.sys_runtime - before.sys_runtime));
	printf("process runtime:   %llu\n", (after.pid_runtime - before.pid_runtime));
	printf("system cpu energy: %f\n", (after.sys_cpu_energy - before.sys_cpu_energy));
	
	//wait for check
	scanf("%c", &c);
}

void Test_mem_power_read(void)
{
	char c;
	int pid = getpid();

	int fd = create_perf_stat_counter(pid);
	unsigned long long before = read_perf_counter(fd);

	dummy();

	unsigned long long after = read_perf_counter(fd);
	printf("values read for process %d are:\n", pid);
	printf("L2 cache misses: %llu\n", (after - before));

	scanf("%c", &c);
}

void Test_disk_power_read(void)
{
	char c;
	int pid = getpid();

	pid_stats_info before, after;

	memset(&before, 0, sizeof(pid_stats_info));
	memset(&after, 0, sizeof(pid_stats_info));

	if(read_pid_io(pid, &before) <= 0)
		printf("ERROR: read_pid_io failed!\n");

	dummy();

	if(read_pid_io(pid, &after) <= 0)
		printf("ERROR: read_pid_io failed!\n");

	printf("values read for process %d are:\n", pid);
	printf("process read bytes:       %llu\n", (after.pid_read_bytes - before.pid_read_bytes));
	printf("process write bytes:      %llu\n", (after.pid_write_bytes - before.pid_write_bytes));
	printf("process cancelled writes: %llu\n", (after.pid_cancelled_writes - before.pid_cancelled_writes));

	scanf("%c", &c);
}
*/
void Test_power_monitor(void)
{
	int i;
	metrics m_resources;
	m_resources.num_metrics = 1;
	m_resources.local_data_storage = 1;
	m_resources.sampling_interval[0] = 2000; // 2s
	strcpy(m_resources.metrics_names[0], "power");

	char server[] = "localhost:3040";
	char platform_id[] = "fangli_laptop";

	char *datapath = mf_start(server, platform_id, &m_resources);
	printf("datapath : %s\n", datapath);
	
	/*do dummy things*/
	for(i = 0; i < 100; i++) {
		dummy();
		//sleep(1);
	}

	mf_end();

}

/*******************************************************************************
 * resources and disk monitor test 
 ******************************************************************************/
void Test_all(void)
{
	int i;
	metrics m_resources;
	m_resources.num_metrics = 3;
	m_resources.local_data_storage = 0;		// to keep the local data files(1) or not(0)
	m_resources.sampling_interval[0] = 1000; // 1s
	strcpy(m_resources.metrics_names[0], "resources_usage");

	m_resources.sampling_interval[1] = 1000; // 1s
	strcpy(m_resources.metrics_names[1], "disk_io");

	m_resources.sampling_interval[2] = 2000; // 1s
	strcpy(m_resources.metrics_names[2], "power");

	char server[] = "localhost:3040";
	char platform_id[] = "fangli_laptop";
	char application_id[] = "dummy";
	char task_id[] = "t1";

	char *datapath = mf_start(server, platform_id, &m_resources);
	printf("datapath : %s\n", datapath);
	
	/*do dummy things*/
	for(i = 0; i < 30; i++) {
		dummy();
		//sleep(1);
	}

	mf_end();

	/* when "dummy" application already exists in database, check with /v1/phantom_mf/workflows, 
	it is possible to send collected metrics to the server */
	
	char *experiment_id = mf_send(server, application_id, task_id, platform_id);
	printf("> application_id : %s\n", application_id);
	printf("> task_id : %s\n", task_id);
	printf("> experiment_id : %s\n", experiment_id);
	free(experiment_id);
}

/*******************************************************************************
 * config read from mf server test 
 ******************************************************************************/
/*
void Test_read_config(void)
{
	char server[] = "localhost:3040";
	char platform_id[] = "fangli_laptop";
	get_config_parameters(server, platform_id);
}
*/

int main(void)
{
	//Test_read_config();

	/* test basic functions */
	//Test_resources_stat_cpu();
	//Test_resources_stat_all_and_calculate();
	//Test_disk_stats_read();

	/* test mf interfaces: mf_start, mf_end */
	//Test_resources_monitor(); 	//only resources monitoring 
	//Test_disk_monitor();		//only disk monitoring

	/* test mf interfaces: mf_start, mf_end, mf_send */	
	Test_all();	//both resources and disk monitoring
	//Test_power_monitor();

	//Test_CPU_power_read();
	//Test_mem_power_read();
	//Test_disk_power_read();
	return 0;
}
