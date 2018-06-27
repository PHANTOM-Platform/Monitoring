/*
* Copyright (C) 2018 University of Stuttgart
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include "mf_debug.h"
#include "publisher.h"

#define SUCCESS 1
#define FAILED 0
/*******************************************************************************
* Variables Declarations
******************************************************************************/
struct curl_slist *headers = NULL;

/*******************************************************************************
* Forward Declarations
******************************************************************************/
int check_URL(char *URL);
int check_message(char *message);
void init_curl(void);
CURL *prepare_publish(char *URL, char *message, char *operation);
CURL *prepare_query(char* URL, char *operation);
// static size_t get_stream_data(void *buffer, size_t size, size_t nmemb, char *stream);

#ifdef NDEBUG
static size_t write_non_data(void *buffer, size_t size, size_t nmemb, void *userp);
#endif


// //definitions for new_query_json
// struct url_data {
// 	size_t size;
// 	char* headercode ;
// 	char* data;
// };

//this function is the callback for colleting the response
// write_data will replace the function get_stream_data
size_t write_data(void *ptr, size_t size, size_t nitems, struct url_data *data) {
	// 'data' is set with function such CURLOPT_HEADERDATA
	size_t index = data->size;
	size_t n = (size * nitems);// received is nitems * size long in 'ptr' NOT ZERO TERMINATED
	data->size += (size * nitems);
	#ifdef DEBUG
		fprintf(stderr, "data at %p size=%ld nitems=%ld\n", ptr, size, nitems);
	#endif

	/* Initial memory allocation */
	char *temp_str = (char *) malloc(data->size + 1);
	if(temp_str==NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return 0;
	}
	int i=0;//instead of data->data = (char*) realloc(data->data, data->size + n);
	while((data->data[i]!='\0') &&(i < index)) {
		temp_str[i]=data->data[i];
		i++;
	}
	temp_str[i]='\0';
	if(data->data!=NULL) free(data->data);
	data->data=temp_str;
	memcpy((data->data + index), ptr, n);
	data->data[data->size] = '\0';

	if(n>9){
		char tempinit[20];
		memcpy(tempinit, ptr, 9);
		tempinit[9] = '\0';
		int rc = strcmp(tempinit, "HTTP/1.1 ");//[0..8] 
		if(rc == 0){ 
			if(data->headercode==NULL)
				data->headercode = (char *) malloc(11); 
			//we wish to cut the first 9 characters which must be: "HTTP/1.1 ", 
			// then take those numeric characters after that 
			i=0;
			while (data->data[i+9]>='0' && data->data[i+9]<='9'){
				data->headercode[i]=data->data[i+9];
				i++;
			} 
			data->headercode[i]='\0';
		}
	}
	return size * nitems;
}

 
// size_t header_callback(void *ptr, size_t size, size_t nitems, struct url_data *data) {
// 	if (strncmp((char *)ptr, "X-Auth-Token:", 13) == 0) { // get Token
// //         strtok((char *)ptr, " ");
// //         data = (strtok(NULL, " \n"));   // token will be stored in data
//     }
//     else if (strncmp((char *)ptr, "HTTP/1.1", 8) == 0) { // get http response code
// //         strtok((char *)ptr, " ");
// //         data = (strtok(NULL, " \n"));   // http response code		
//     } 
// 	return size * nitems;
// }


/* Callback function to get stream data during writing */
// static size_t get_stream_data(void *buffer, size_t size, size_t nmemb, char *stream)
// {
// 	size_t total = size * nmemb;
// 	/* re-allocate string length */
// 	stream = realloc(stream, total + 1);
// 	if(stream == NULL) {
// 		return 0;
// 	}
//
// 	memcpy(stream, buffer, total);
// 	stream[total] = '\0';
// 	return total;
// }


/**********************************************************
 * FUNCTION DECLARATIONS
 *****************************************************/
#include <malloc.h>
//Function for increase dynamically a string concatenating strings at the end
//It free the memory of the first pointer if not null
char* concat_and_free(char **s1, const char *s2)
{
	char *result = NULL;
	unsigned int new_lenght= strlen(s2)+1; //+1 for the null-terminator;
	if(*s1 != NULL){
		new_lenght+= strlen(*s1);//current lenght
		if(new_lenght> malloc_usable_size(*s1)){
			result = (char *) malloc(new_lenght);
			strcpy(result, *s1);
			free(*s1);
		}else{
			result = *s1; 
		} 
	}else{
		result = (char *) malloc(new_lenght);
		result[0]='\0';
	}
	//in real code you would check for errors in malloc here 
	strcat(result, s2);
	return result;
}



/* send query to the given URL, read back the response string
return 1 on success; otherwise return 0 */
int new_query_json(char *URL, struct url_data *response, char *operation)
{
	response->size=0;
	response->data = NULL;
	response->headercode = NULL;
	
	struct url_data data;
	data.size = 0;
	data.data = malloc(5192); /* reasonable size initial buffer */
	if(NULL == data.data) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	}
	data.data[0] = '\0';
	data.headercode = malloc(5192); /* reasonable size initial buffer */
	if(NULL == data.headercode) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	}
	data.headercode[0] = '\0';
	
	struct url_data rescode;
	rescode.size = 0;
	rescode.data = malloc(5192); /* reasonable size initial buffer */
	if(NULL == rescode.data) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	}
	rescode.data[0] = '\0';
	rescode.headercode = malloc(5192); /* reasonable size initial buffer */
	if(NULL == rescode.headercode) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	}	
	rescode.headercode[0] = '\0';

	 	
	 

	if (!check_URL(URL)) {
		return FAILED;
	}

	CURL *curl = prepare_query(URL, operation);
	if (curl == NULL) {
		free(data.data);
		free(data.headercode); 
		free(rescode.data);
		free(rescode.headercode);
		return FAILED;
	}

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	
 
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data);//header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rescode); // set userdata in callback function	
	CURLcode response_code = curl_easy_perform(curl);
// 	printf("\n RESCODE is %s\n", rescode.headercode);
// 	printf("\n RESCODE.data is %s\n", rescode.data);	
	response->headercode=rescode.headercode;
	free(rescode.data); rescode.data=NULL;
	
	
	if (response_code != CURLE_OK) {
		const char *error_msg = curl_easy_strerror(response_code);
		log_error("ERROR query %s", error_msg);
// 		printf("ERROR!! : query with %s failed.\n", error_msg);
		free(data.data);
		free(data.headercode); 
		free(rescode.headercode);
		return FAILED;
	}

	//curl_slist_free_all(headers);/* free the list again */
	curl_easy_cleanup(curl);
	if( data.data[0]=='\0') {
		printf("ERROR!! : query with %s failed.\n", URL);
		free(data.data);
		free(data.headercode); 
		free(rescode.headercode);
		return FAILED;
	}
	
	if(response->data !=NULL) free(response->data);
	response->data=data.data;
	response->headercode=rescode.headercode;
 
	free(data.headercode); 
	
	if(data.data == NULL) {
		return FAILED;
	}
	return SUCCESS;
}


/* send query to the given URL, read back the response string
return 1 on success; otherwise return 0 */
// int query_json(char *URL, char *response_str, char *operation)
// {
// 	if (!check_URL(URL)) {
// 		return FAILED;
// 	}
//
// 	CURL *curl = prepare_query(URL, operation);
// 	if (curl == NULL) {
// 		return FAILED;
// 	}
//
// 	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_stream_data);
// 	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_str);
// 	CURLcode response_code = curl_easy_perform(curl);
// 
// 	if (response_code != CURLE_OK) {
// 		const char *error_msg = curl_easy_strerror(response_code);
// 		log_error("create_new_experiment %s", error_msg);
// 		return FAILED;
// 	}
//	//curl_slist_free_all(headers);/* free the list again */
// 	curl_easy_cleanup(curl);
//
// 	if(response_str == NULL) {
// 		return FAILED;
// 	}
// 	return SUCCESS;
// }

/* json-formatted data publish using libcurl
return 1 on success; otherwise return 0 */
int publish_json(char *URL, char *message)
{
	struct url_data rescode;
	rescode.size=0;
	rescode.data = malloc(5192); /* reasonable size initial buffer */
	if(NULL == rescode.data) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	}
	rescode.data[0] = '\0';
	
	 

	rescode.headercode = (char *) malloc(5120); /* reasonable size initial buffer */
	if(NULL == rescode.headercode) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	} 
	rescode.headercode[0] = '\0';	
	
	
	
	if (!check_URL(URL) || !check_message(message)) {
		return FAILED;
	}
	char operation[]="POST";
	CURL *curl = prepare_publish(URL, message, operation);
	if (curl == NULL) {
		return FAILED;
	}

	#ifdef NDEBUG
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_non_data);//
	#endif

 
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data);//header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rescode);  // set userdata in callback function
	CURLcode response_code = curl_easy_perform(curl);
// 	printf("\n RESCODE is %s\n", rescode.data);
	free(rescode.data); rescode.data=NULL;
	
	if (response_code != CURLE_OK) {
		const char *error_msg = curl_easy_strerror(response_code);
		log_error("publish(char *, Message) %s", error_msg);
		return FAILED;
	}

	//curl_slist_free_all(headers);/* free the list again */
	curl_easy_cleanup(curl);
	return SUCCESS;
}

/* publish a file with given filename and URL 
each line is read and combined with the given static string, formatted into json, and sent via libcurl
return 1 on success; otherwise return 0 */
int publish_file(char *URL, char *static_string, char *filename)
{
	if (!check_URL(URL) || !check_message(static_string) || !check_message(filename)) {
		return FAILED;
	}
	/*open the file, which contains data for publishing */
	int i = 0;
	FILE *fp;
	char line[320];
	char *message = calloc(10 * 320, sizeof(char));
	char operation[]="POST";
	
	/* int curl with meaningless message */
	CURL *curl = prepare_publish(URL, message, operation);
	if (curl == NULL) {
		return FAILED;
	}
	#ifdef NDEBUG
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_non_data);
	#endif

	fp = fopen(filename, "r");
	if(fp == NULL) {
		log_error("Could not open file %s\n", filename);
		return FAILED;
	}

	/* read the lines in the file and send the message for each 10 lines */
	while(fgets(line, 320, fp) != NULL) {
		line[strlen(line) - 1] = '\0';
		switch(i) {
			case 0:
				sprintf(message, "[{%s, %s}", static_string, line);
				i++;
				break;
			case 9:
				sprintf(message + strlen(message), ",{%s, %s}]", static_string, line);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long ) strlen(message));
				CURLcode response_code = curl_easy_perform(curl);

				if (response_code != CURLE_OK) {
					const char *error_msg = curl_easy_strerror(response_code);
					log_error("publish(char *, Message) %s", error_msg);
					return FAILED;
				}
				/* reset i and message for following sending */
				i = 0;
				memset(message, '\0', 10*320);
				break;
			default:
				sprintf(message + strlen(message), ",{%s, %s}", static_string, line);
				i++;
				break;
		}
	}
	/* send the final few lines in the file */
	if(i > 0) {
		strcat(message, "]");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long ) strlen(message));
		CURLcode response_code = curl_easy_perform(curl);

		if (response_code != CURLE_OK) {
			const char *error_msg = curl_easy_strerror(response_code);
			log_error("publish(char *, Message) %s", error_msg);
			return FAILED;
		}
	}
	
	/* clean the curl handle */	
	//curl_slist_free_all(headers);/* free the list again */
	curl_easy_cleanup(curl);
	if(message!=NULL) free(message);
	return SUCCESS;
}

/**
* It can not be register experiments and workflows 
* read back the path to the workflow, after send the msg
* return 1 on success; otherwise return 0 
* 
* Example of inputs for register WORKFLOWS:
*  URL: localhost:3033/v1/phantom_mf/workflows/demo
*  msg: {"application":"demo", "author": "Guy", "optimization": "Time",
*       "tasks":[{"device":"demo_desktop", "exec":"hello_world", "cores_nr": "1"}]}
*
* Manually can be done from command line like: 
* curl -H "Content-Type: application/json" -XPUT ${server}:${port}/v1/phantom_mf/workflows/${appid} -d '{
* "application":"'"${appid}"'","author":"Random Guy","optimization":"Time","tasks":[{"device":"'"${regplatformid}"'","exec":"'"${task}"'","cores_nr": "1"}]}'
*
* Example of register a new EXPERIMENT for specific application
* read back the generated experiment_id, after send the msg
* return 1 on success; otherwise return 0 
* 
* Example of inputs:
*  URL: localhost:3033/v1/phantom_mf/experiments/demo
*  msg: {"application":"demo", "task": "hello_world", "host": "demo_desktop"}
* example of return output on experiment_id
*  AWQijfEZZH5jRvHSyXlY
* 
* Manually can be done from command line like:
* curl -H "Content-Type: application/json" -XPOST ${server}:${port}/v1/phantom_mf/experiments/${appid} -d '{
* "application": "'"${appid}"'", "task": "'"${task}"'", "host": "'"${regplatformid}"'"}'
*/ 
int query_message_json(char *URL, char *message, struct url_data *response, char *operation)
{
	struct url_data data; 
	data.size = 0;
	data.data = (char *) malloc(5120); /* reasonable size initial buffer */
	if(NULL == data.data) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	} 
	data.data[0] = '\0';
	data.headercode = (char *) malloc(5120); /* reasonable size initial buffer */
	if(NULL == data.headercode) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	} 
	data.headercode[0] = '\0';
	
	struct url_data rescode;
	rescode.size=0;
	rescode.data = malloc(5192); /* reasonable size initial buffer */
	if(NULL == rescode.data) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	}
	rescode.data[0] = '\0';
	rescode.headercode = malloc(5192); /* reasonable size initial buffer */
	if(NULL == rescode.headercode) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	}
	rescode.headercode[0] = '\0';
	
	response->data=NULL;
	
	if (!check_URL(URL) || !check_message(message)) {
		free(data.data);
		free(data.headercode);
		free(rescode.data);
		free(rescode.headercode);
		return FAILED;
	}
	
	CURL *curl = prepare_publish(URL, message, operation);
	if (curl == NULL) {
		free(data.data);
		free(data.headercode);
		free(rescode.data);
		free(rescode.headercode);
		return FAILED;
	}
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // the functions get_stream_data seems was not correct.
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
 
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data);//header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rescode);  // set userdata in callback function
	CURLcode response_code = curl_easy_perform(curl);
// 	printf("\n URL is %s\n", URL);
// 	printf("\n message is %s\n", message);
// 	printf("\n RESCODE is %s\n", rescode.headercode);
	free(rescode.data); rescode.data=NULL;
	
	if (response_code != CURLE_OK) {
		const char *error_msg = curl_easy_strerror(response_code);
		log_error("ERROR query_message_json %s", error_msg);
		free(data.data);
		free(data.headercode);
		free(rescode.data);
		free(rescode.headercode);
		return FAILED;
	}

	//curl_slist_free_all(headers);/* free the list again */
	curl_easy_cleanup(curl);

	if( data.data[0]=='\0') {
		printf("ERROR!! : query with %s failed.\n", URL);
		free(data.data);
		free(data.headercode);
		free(rescode.data);
		free(rescode.headercode);
		return FAILED;
	}
	if(response->data !=NULL) free(response->data);
	if(response->headercode !=NULL) free(response->headercode);
	response->data=data.data;
	response->headercode =rescode.headercode;
	
 
	free(data.headercode);
	free(rescode.data); 
	
	if(response->data == NULL) {
		return FAILED;
	}
	return SUCCESS;
}

/* create new experiment for specific application
read back the generated experiment_id, after send the msg
return 1 on success; otherwise return 0 */
// int create_new_experiment(char *URL, char *message, char *experiment_id)
// {
// 	//You should use query_message_json instead of this function create_new_experiment !!
// 	if (!check_URL(URL) || !check_message(message)) {
// 		return FAILED;
// 	}
// 	CURL *curl = prepare_publish(URL, message, "POST");
// 	if (curl == NULL) {
// 		return FAILED;
// 	}
// 	
// 	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_stream_data);
// 	curl_easy_setopt(curl, CURLOPT_WRITEDATA, experiment_id);
// 	CURLcode response = curl_easy_perform(curl);
// 
// 	if (response != CURLE_OK) {
// 		const char *error_msg = curl_easy_strerror(response);
// 		log_error("create_new_experiment %s", error_msg);
// 		return FAILED;
// 	}
//	//curl_slist_free_all(headers);/* free the list again */
// 	curl_easy_cleanup(curl);
// 	return SUCCESS;
// }

/* Check if the url is set 
return 1 on success; otherwise return 0 */
int check_URL(char *URL)
{
	if (URL == NULL || *URL == '\0') {
		const char *error_msg = "URL not set.";
		log_error("publish(char *, Message) %s", error_msg);
		return FAILED;
	}
	return SUCCESS;
}

/* Check if the message is set 
return 1 on success; otherwise return 0 */
int check_message(char *message)
{
	if (message == NULL || *message == '\0') {
		const char *error_msg = "message not set.";
		log_error("publish(char *, Message) %s", error_msg);
		return FAILED;
	}
	return SUCCESS;
}

/* Initialize libcurl; set headers format */
void init_curl(void)
{
	if (headers != NULL ) {
		return;
	}
	curl_global_init(CURL_GLOBAL_ALL);
// 	headers = curl_slist_append(headers, string("X-Auth-Token: " + token).c_str()); 
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charsets: utf-8");
}

/* Prepare for using libcurl with message */
CURL *prepare_publish(char *URL, char *message, char *operation)
{
	init_curl();
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, URL);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long ) strlen(message));
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, operation); /* PUT, POST, ... */
	 
	#ifdef DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	#endif

	return curl;
}

/* Prepare for using libcurl without message */
CURL *prepare_query(char* URL, char *operation)
{
	init_curl();
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, URL);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, operation); /* GET, PUT... */
	
	#ifdef DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	#endif

	return curl;
}

/* Callback function for writing with libcurl */
#ifdef NDEBUG
static size_t write_non_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	return size * nmemb;
}
#endif

