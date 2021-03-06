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

#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include "thread_handler.h"	//functions like startThreads()...
#include "mf_debug.h"		//functions like log_error(), log_info()...
#include "mf_parser.h" 		//functions like mfp_parse()...
#include "publisher.h"		//functions like create_new_experiment()...
#include "main.h"
#include "mf_util.h"

#define SUCCESS 0
#define FAILURE 1

/*******************************************************************************
* External Variables Declarations
******************************************************************************/
char *pwd=NULL;
char *confFile=NULL;
char *application_id=NULL;
char *experiment_id=NULL;
char *task_id=NULL;
char *platform_id=NULL;
char *token=NULL;
char *metrics_publish_URL=NULL;
char *logFileName=NULL;
FILE *logFile;		//declared as extern in mf_debug.h


/*******************************************************************************
* Variable Declarations
******************************************************************************/
char *name;
int pwd_is_set = 0;

/*******************************************************************************
* Forward Declarations
******************************************************************************/
void set_pwd(void);
void createLogFile(void);
int writeTmpPID(void);
int prepare(const char *token);

/*******************************************************************************
* Functions implementation
******************************************************************************/
/* everything starts here */
int main(int argc, char* argv[]) {
	extern char *optarg;
	int c;
	int err = 0, help = 0;
	char server_name[128] = {'\0'};
	
	/* init arguments */
	pwd = calloc(256, sizeof(char));pwd[0]='\0';
	name = malloc(256* sizeof(char));name[0]='\0'; 			//name of the tmp pid file
	application_id = calloc(128, sizeof(char));application_id[0]='\0';		//should be given as input
	experiment_id = malloc(128* sizeof(char));experiment_id[0]='\0';		//we register a new one
	task_id = malloc(128* sizeof(char));task_id[0]='\0';			//should be generated by mf_server
	confFile = malloc(256* sizeof(char));confFile[0]='\0';

	/* assigns the current working directory to the variable "pwd" */
	set_pwd();

	/* create log file, use log_error(), log_info() and log_warn() afterwards */
	createLogFile();
// 	printf("\n **** pwd %s ******\n",pwd);
	/* write PID to a file ("tmp_pid") in `pwd`; can be used to kill the agent */
	if(writeTmpPID() == FAILURE)
		exit(FAILURE);

	/* parse command-line arguments */
	while ((c = getopt(argc, argv, "a:t:h")) != -1)
		switch (c) {
		case 'a':
			strcpy(application_id, optarg);
			break;
		case 't':
			strcpy(task_id, optarg);
			break;
		case 'h':
			help = 1;
			break;
		case '?':
			err = 1;
			break;
	}

	/* print usage */
	if (err || help) {
		char usage[256] = {'\0'};
		sprintf(usage, "usage: %s [-a application_id] [-t task_id] [-h help]\n", argv[0]);
		log_error("%s", usage);
		exit(FAILURE);
	}
	
	/* set the configuration file */
	sprintf(confFile, "%s/%s", pwd, "mf_config.ini");
// 	log_info("Configuration taken from: %s.\n", confFile);
	printf("Configuration taken from: %s.\n", confFile);

	/* try to parse configuration file */
	if (mfp_parse(confFile) == FAILURE) {
		/* could not parse configuration file */
		fprintf(logFile, "[ERROR] Stopping service...could not parse configuration.\n");
		exit(FAILURE);
	}


//we not need to register an experiment of a prvious register workflow
// 	/* prepare metrics_publish_URL and platform_id, based on mf_config.ini;
// 	send a initialization msg to the server and create a new experiment with unique experiment_id*/
//we not want to register new experiments, only monitor the device and update the resource manager	
if(1==0){
	if (prepare(token) == FAILURE) {
		fprintf(logFile, "[ERROR] Stopping service...could not prepare URLs for sending metrics to server.\n");
		printf("[ERROR] Stopping service...could not prepare URLs for sending metrics to server.\n");
		exit(FAILURE);
	}
}else{
// 	curl -s -H "Authorization: OAuth ${mytoken}" -H "Content-Type: multipart/form-data" -XPOST -F "UploadJSON=@../web/device_mod.json" http://${server}:${resourcemanager_port}/update_device;
// 	curl -s -XGET ${server}:${resourcemanager_port}/_flush > /dev/null;
	metrics_publish_URL = calloc(256, sizeof(char));
	/* get server */
	mfp_get_value("generic", "resourcemng", server_name);
	/* get metrics send url */
	sprintf(metrics_publish_URL, "%s/update_status_json", server_name);
	/* get platform_id */
	platform_id= calloc(128, sizeof(char));application_id[0]='\0';		//should be given as input
	mfp_get_value("generic", "platform_id", platform_id);

	/* get token */
	token= calloc(256, sizeof(char));application_id[0]='\0';		//should be given as input	
	mfp_get_value("generic", "token", token);
}

	/* start threads for plugins, implementation in thread_handler.c */
	if(startThreads( metrics_publish_URL,token, platform_id) == FAILURE) {
		fprintf(logFile, "[ERROR] Stopping service...could not start the threads.\n");
		exit(FAILURE);
	}

	/* monitoring ends */
	fprintf(logFile, "[INFO] Stopping service ...\n");
	fclose(logFile);

	if(name[0]!='\0') unlink(name); // delete the file which stores the tmp pid
	if(pwd!=NULL) free(pwd);
	if(name!=NULL) free(name);
	if(application_id!=NULL) free(application_id);
	if(experiment_id!=NULL) free(experiment_id);
	if(task_id!=NULL) free(task_id);
	if(metrics_publish_URL!=NULL) free(metrics_publish_URL);
	if(platform_id!=NULL) free(platform_id);
	if(logFileName!=NULL) free(logFileName);
	if(confFile!=NULL) free(confFile);

	mfp_parse_clean();
	exit(SUCCESS);
}

/* assigns the current working directory to the variable "pwd" */
void set_pwd(void) {
	if (pwd_is_set)
		return;
	char buf_1[256] = {'\0'};
	char buf_2[256] = {'\0'};

	readlink("/proc/self/exe", buf_1, 200);
	memcpy(buf_2, buf_1, (strlen(buf_1)+1) * sizeof(char));

	/* extract path folder of executable from it's path */
	char *lastslash = strrchr(buf_2, '/');
	int ptr = lastslash - buf_2;

	memcpy(pwd, buf_2, ptr);
	pwd_is_set = 1;
}

/* Initializing and creating the log file */
void createLogFile(void) {
	int errnum;
	if (!pwd_is_set)
		set_pwd();
	
	logFileName = calloc(300, sizeof(char));	//extern variable
	char logFileFolder[300] = { '\0' };
	time_t curTime;
	time(&curTime);
	struct tm *time_info = localtime(&curTime);
	char timeForFile[50];

	strftime(timeForFile, 50, "%F-%T", time_info);
	sprintf(logFileFolder, "%s/log", pwd);
	sprintf(logFileName, "%s/log/log-%s", pwd, timeForFile);
	fprintf(stderr, "using logfile: %s\n", logFileName);

	struct stat st = { 0 };
	if (stat(logFileFolder, &st) == -1)
		mkdir(logFileFolder, 0700);
	logFile = fopen(logFileName, "a");
	if (logFile == NULL) {
		errnum = errno;
		fprintf(stderr, "Could not create log file: %s\n", logFileName);
		fprintf(stderr, "Error creating log: %s\n", strerror(errnum));
	} else {
		log_info("Starting service at %s...\n", timeForFile);
	}
}

/* write PID to a file (named as "tmp_pid") in `pwd` */
int writeTmpPID(void) {
	if (!pwd_is_set)
		set_pwd();
	strcpy(name, pwd);
	strcat(name, "/tmp_pid");

	int pid = getpid();
	FILE *tmpFile = fopen(name, "w");
	if (tmpFile == NULL) {
		log_error("Failed to create file %s to store the PID.\n", name);
		return FAILURE;
	} else {
		fprintf(tmpFile, "%d", pid);
		fclose(tmpFile);
	}
	return SUCCESS;
}

/* prepare metrics_publish_URL and platform_id, based on mf_config.ini;
send a initialization msg to the server and create a new experiment with unique experiment_id-> response.data*/
int prepare(const char *token) {
	char server_name[128] = {'\0'};
	metrics_publish_URL = calloc(256, sizeof(char));
	platform_id = calloc(128, sizeof(char));

	/* get server */
	mfp_get_value("generic", "server", server_name);
	/* get metrics send url */
	sprintf(metrics_publish_URL, "%s/phantom_mf/metrics", server_name);
	/* get platform_id */
	mfp_get_value("generic", "platform_id", platform_id);

	/* by default, no application_id and task_id is given, 
	therefore set application_id to infrastructure; task_id to the platform_id */
	if(application_id[0] == '\0' || task_id[0] == '\0') {
		strcpy(application_id, "infrastructure");
		strcpy(task_id, platform_id);
	}

	/* create an new experiment by sending msg to mf_server */
	char *URL = NULL;
	char *msg = NULL;

	msg=concat_and_free(&msg, "{\"application\":\"");
	msg=concat_and_free(&msg, application_id);
	if(task_id==NULL){
		printf(" mf_send task_id is NULL\n");
	}else{
		msg=concat_and_free(&msg, "\", \"task\": \"");
		msg=concat_and_free(&msg, task_id);
	}
	if(platform_id==NULL){
		printf(" mf_send platform_id is NULL\n");
	}else{
		msg=concat_and_free(&msg, "\", \"host\": \"");
		msg=concat_and_free(&msg, platform_id);
	}
	msg=concat_and_free(&msg, "\"}");
	
	if(server_name!=NULL){
		URL=concat_and_free(&URL, server_name);
	}else{
		printf(" server_name is NULL\n");return FAILURE;
	}
	URL=concat_and_free(&URL, "/phantom_mf/experiments/");
	URL=concat_and_free(&URL, application_id);
	if(token==NULL){
		printf(" mf_send token is NULL\n");return FAILURE;
	}
	struct url_data response;
	response.size=0;
	response.data=NULL;
	response.headercode=NULL;


// 	create_new_experiment(experiments_URL, msg, experiment_id); --> new experiment_id is response.data
	const char operation[]="POST";
// 	const char filepath[]="";//not upload any file
// 	printf("### -X%s %s -d %s\n", operation,URL, msg);
	if(query_message_json(URL, msg, NULL,  &response, operation, token)==FAILURE){
		printf("error on query_message_json\n");
		return FAILURE;
	}; 
	if(response.data[0] == '\0') {
		log_error("Cannot create new experiment for application %s\n", application_id);
		return FAILURE;
	}
// 	printf(" code %s\n",response.headercode);
	if(response.headercode[0]=='4'){
		return FAILURE;
	}
	strcpy(experiment_id,response.data);
	printf("> application_id : %s\n", application_id);
	printf("> task_id : %s\n", task_id);
	printf("> response.data : %s\n", response.data); //this is the experiment_id
	
	/* close and reopen logFile */
	fclose(logFile);
	logFile = fopen(logFileName, "a");
	return SUCCESS;
}
