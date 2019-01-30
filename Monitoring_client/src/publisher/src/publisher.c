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
#include "mf_util.h"
#include "publisher.h"
#include <malloc.h>
#include <sys/stat.h>

// From http://curl.haxx.se/libcurl/c/curl_global_init.html, I see 
// 
// "This function is not thread safe. You must not call it when any other thread
// in the program (i.e. a thread sharing the same memory) is running. This doesn't
// just mean no other thread that is using libcurl. Because curl_global_init() 
// calls functions of other libraries that are similarly thread unsafe, it could
// conflict with any other thread that uses these other libraries. "
//
//
// void curl_global_cleanup(void);
// DESCRIPTION: This function releases resources acquired by curl_global_init.

#define SUCCESS 0
#define FAILED 1

#define headercode_char_size 11
/*******************************************************************************
* Variables Declarations
******************************************************************************/
struct curl_slist *headers = NULL;

/*******************************************************************************
* Forward Declarations
******************************************************************************/
int check_URL(const char *URL);
int check_message(const char *message);
void init_curl(const char *token);
CURL *prepare_publish(char *URL, char *message, FILE *send_fp, char *operation, const char *token);
CURL *prepare_query(char* URL, char *operation, const char *token);
// static size_t get_stream_data(void *buffer, size_t size, size_t nmemb, char *stream);

/** Callback function for writing with libcurl */
#ifdef NDEBUG
static size_t write_non_data(void *buffer, size_t size, size_t nmemb, void *userp) {
	return size * nmemb;
}
#endif

// //definitions for new_query_json
// struct url_data {
// 	size_t size;
// 	char* headercode;
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
	unsigned int i=0;//instead of data->data = (char*) realloc(data->data, data->size + n);
	while((data->data[i]!='\0') &&(i < (unsigned int) index)) {
		temp_str[i]=data->data[i];
		i++;
	}
	temp_str[i]='\0';
	if(data->data!=NULL) free(data->data); 
	data->data=NULL;
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
				data->headercode = (char *) malloc(headercode_char_size); 
			//we wish to cut the first 9 characters which must be: "HTTP/1.1 ", 
			// then take those numeric characters after that 
			i=0;
			while (data->data[i+9]>='0' && data->data[i+9]<='9' && i<headercode_char_size){
				data->headercode[i]=data->data[i+9];
				i++;
			} 
			data->headercode[i]='\0';
		}
	}
	return size * nitems;
}

// size_t header_callback(void *ptr, size_t size, size_t nitems, struct url_data *data) {
// 	if(strncmp((char *)ptr, "X-Auth-Token:", 13) == 0) { // get Token
// //         strtok((char *)ptr, " ");
// //         data = (strtok(NULL, " \n"));   // token will be stored in data
//     }
//     else if(strncmp((char *)ptr, "HTTP/1.1", 8) == 0) { // get http response code
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
// 	memcpy(stream, buffer, total);
// 	stream[total] = '\0';
// 	return total;
// }

/**********************************************************
 * FUNCTION DECLARATIONS
 *****************************************************/
void free_data_struc(struct url_data *data){
	if(data->data!=NULL) free(data->data);
	if(data->headercode!=NULL) free(data->headercode);
	data->data=NULL;
	data->headercode=NULL;
}

int reserve_data_struc(struct url_data *data){
	data->size = 0;
	data->data = malloc(5192); /* reasonable size initial buffer */
	if(NULL == data->data) {
		data->data=NULL;
		data->headercode=NULL;
		return FAILED;
	}
	data->data[0] = '\0';
	data->headercode = malloc(headercode_char_size); /* reasonable size initial buffer */
	if(NULL == data->headercode) {
		free_data_struc(data);
		return FAILED;
	}
	data->headercode[0]= '\0';
	return SUCCESS;
}

/** send query to the given URL, read back the response string
* @return 1 on success; otherwise return 0
* if the token is NULL or empty string, will procedd as not token provided or required*/
int new_query_json(char *URL, struct url_data *response, char *operation, const char *token) {
	struct url_data data;
	struct url_data rescode;
	response->size=0;
	response->data = NULL;
	response->headercode = NULL;

	if(check_URL(URL)!=SUCCESS)
		return FAILED;

	if(reserve_data_struc(&data) == FAILED) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	}
	if(reserve_data_struc(&rescode) == FAILED) { //***********************************************************
		fprintf(stderr, "Failed to allocate memory.\n");
		free_data_struc(&data);
		return FAILED;
	}

	CURL *curl = prepare_query(URL, operation, token);
	if(curl == NULL) {
		free_data_struc(&data);
		free_data_struc(&rescode);
		fprintf(stderr, "Failed to allocate memory.\n");
		return FAILED;
	}

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data);//header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rescode); // set userdata in callback function
	CURLcode response_code = curl_easy_perform(curl);//***********************************************************

	if(response_code != CURLE_OK) {
		const char *error_msg = curl_easy_strerror(response_code);
		log_error("ERROR query %s", error_msg);
		free_data_struc(&data);
		free_data_struc(&rescode);
		return FAILED;
	}

	//curl_slist_free_all(headers);/* free the list again */
	curl_easy_cleanup(curl);
	if( data.data[0]=='\0') {
		printf("ERROR!! : query with %s failed.\n", URL);
		free_data_struc(&data);
		free_data_struc(&rescode);
		return FAILED;
	}
	
	if(response->data !=NULL) free(response->data);
		response->data=NULL;
	if(response->headercode !=NULL) free(response->headercode);
		response->headercode=NULL;
 
	if(rescode.data!=NULL) free(rescode.data);
		rescode.data=NULL;
	if(rescode.headercode!=NULL) free(data.headercode); 
	data.headercode=NULL;
	
	if(data.data == NULL){
		printf("data.data = NULL\n");
		free_data_struc(&data);
		free_data_struc(&rescode);
		return FAILED;
	}
	response->data=data.data;
	response->headercode=rescode.headercode;
	return SUCCESS;
}
 

/* send query to the given URL, read back the response string
return 1 on success; otherwise return 0 */
// int query_json(char *URL, char *response_str, char *operation) {
// 	if(check_URL(URL)!=SUCCESS)
// 		return FAILED;
// 	CURL *curl = prepare_query(URL, operation, NULL);
// 	if(curl == NULL)
// 		return FAILED;
// 	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_stream_data);
// 	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_str);
// 	CURLcode response_code = curl_easy_perform(curl);
// 	if(response_code != CURLE_OK) {
// 		const char *error_msg = curl_easy_strerror(response_code);
// 		log_error("create_new_experiment %s", error_msg);
// 		return FAILED;
// 	}
//	//curl_slist_free_all(headers);/* free the list again */
// 	curl_easy_cleanup(curl);
// 	if(response_str == NULL)
// 		return FAILED;
// 	return SUCCESS;
// }


/** json-formatted data publish using libcurl
* return 1 on success; otherwise return 0 */
int publish_json(char *URL, char *message, const char *token) {
	struct url_data rescode;
	if(reserve_data_struc(&rescode)==FAILED)
		return FAILED;
	if(check_URL(URL)!=SUCCESS || check_message(message)!=SUCCESS)
		return FAILED;
	char operation[]="POST";
	CURL *curl = prepare_publish(URL, message, NULL, operation, token);
	if(curl == NULL)
		return FAILED;
	#ifdef NDEBUG
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_non_data);//
	#endif
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data);//header_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rescode);  // set userdata in callback function
	CURLcode response_code = curl_easy_perform(curl);
// 	printf("\n RESCODE is %s\n", rescode.data);
	free(rescode.data); rescode.data=NULL;
	if(response_code != CURLE_OK) {
		const char *error_msg = curl_easy_strerror(response_code);
		log_error("publish(char *, Message) %s", error_msg);
		return FAILED;
	}
	//curl_slist_free_all(headers);/* free the list again */
	curl_easy_cleanup(curl);
	return SUCCESS;
}

/** publish a file with given filename and URL
* each line is read and combined with the given static string, formatted into json, and sent via libcurl
* return 1 on success; otherwise return 0 */
int publish_file(char *URL, char *static_string, char *filename, const char *token) {
	if(check_URL(URL)!=SUCCESS || check_message(static_string)!=SUCCESS || check_message(filename)!=SUCCESS)
		return FAILED;
	/*open the file, which contains data for publishing */
	int i = 0;
	unsigned int max_buffer= 10*2048;
	unsigned int adding_string_len;
	CURLcode response_code;
	FILE *fp;
	char line[2048];
	char *message = (char *) malloc(max_buffer *sizeof(char));
	message[0]='\0';
	char operation[]="POST";
	/* int curl with meaningless message */
	CURL *curl = prepare_publish(URL, message, NULL, operation, token);
	if(curl == NULL)
		return FAILED;
	#ifdef NDEBUG
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_non_data);
	#endif
	fp = fopen(filename, "r");
	if(fp == NULL) {
		log_error("Could not open file %s\n", filename);
		return FAILED;
	}
	/* read the lines in the file and send the message for each 10 lines */
	while(fgets(line, 2048, fp) != NULL) {
		line[strlen(line) - 1] = '\0';
		adding_string_len=strlen(static_string)+strlen(line);
		switch(i) {
			case 0:
				if(adding_string_len> max_buffer){
					max_buffer=adding_string_len;
					message = (char *) realloc(message,max_buffer  * sizeof(char));
					if(message==NULL){
						printf("Error allocating memory at publish_file\n");fflush(stdout);
						exit(0);
					}
				}
				sprintf(message, "[{%s, %s}", static_string, line);
				i++;
				break;
			case 9:
				if(strlen(message)+adding_string_len> max_buffer){
					max_buffer+=adding_string_len;
					message = (char *) realloc(message, max_buffer * sizeof(char));
					if(message==NULL){
						printf("Error allocating memory at publish_file\n");fflush(stdout);
						exit(0);
					}
				}
				sprintf(message + strlen(message), ",{%s, %s}]", static_string, line);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long ) strlen(message));
				  response_code = curl_easy_perform(curl);
				if(response_code != CURLE_OK) {
					const char *error_msg = curl_easy_strerror(response_code);
					log_error("publish(char *, Message) %s", error_msg);
					return FAILED;
				}
				/* reset i and message for following sending */
				i = 0;
				message[0]='\0';//memset(message, '\0', 10*320);
				break;
			default:
				if(strlen(message)+adding_string_len> max_buffer){
					max_buffer+=adding_string_len;
					message = (char *) realloc(message, max_buffer * sizeof(char));
					if(message==NULL){
						printf("Error allocating memory at publish_file\n");fflush(stdout);
						exit(0);
					}
				}
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
		  response_code = curl_easy_perform(curl);

		if(response_code != CURLE_OK) {
			const char *error_msg = curl_easy_strerror(response_code);
			log_error("publish(char *, Message) %s", error_msg);
			fclose(fp);
			return FAILED;
		}
	}
	/* clean the curl handle */
	//curl_slist_free_all(headers);/* free the list again */
	curl_easy_cleanup(curl);
	if(message!=NULL) free(message);
	message=NULL;
	fclose(fp);
	return SUCCESS;
}


//file fp is already opened as read operation, or assing to NULL if not file will be transmitted 
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
int query_message_json(char *URL, char *message,const char *filenamepath, struct url_data *response, char *operation, const char *token) {
	struct curl_slist *headers = NULL;
	struct url_data data;
	if(reserve_data_struc(&data)==FAILED)
		return FAILED;
	struct url_data rescode;
	if(reserve_data_struc(&rescode)==FAILED){
		free_data_struc(&data);
		return FAILED;
	}

	response->data=NULL;
	if(check_URL(URL)!=SUCCESS) {
		free(data.data); data.data=NULL;
		free(data.headercode); data.headercode=NULL;
		free(rescode.data); rescode.data=NULL;
		free(rescode.headercode); rescode.headercode=NULL;
		return FAILED;
	}
	if(filenamepath!= NULL){
		headers=NULL;
		// 	curl_slist_free_all(headers);/* free the list again */
		// 	curl_global_init(CURL_GLOBAL_ALL);kkj
		if(token !=NULL){
			if(token[0]!='0'){
				const char authorization_header[]="Authorization: OAuth ";
				unsigned int size=strlen(authorization_header) + strlen(token)+1;
				char *newheader = (char *) malloc(size);
				if(newheader==NULL) {
					printf("Failed to allocate memory.\n");
					exit(1);
				}
				newheader[0]='0';
				strcpy(newheader,authorization_header);
				strcat(newheader,token);
				headers = curl_slist_append(headers, newheader);
				free(newheader);
			}
		}
	// 	headers = curl_slist_append(headers, string("X-Auth-Token: " + token).c_str());
		headers = curl_slist_append(headers, "Expect: 100-continue");
		headers = curl_slist_append(headers, "Content-type: multipart/form-data");
	// 	headers = curl_slist_append(headers, "Accept: application/json");
	// 	headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "charsets: utf-8");
	}else{
		init_curl(token);//this defined the headers
	}

	CURL *curl = NULL;
	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	if(filenamepath!= NULL){
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);//not using signals to notify timeouts on requests and seems not to work fine with multi-threading
		curl_easy_setopt(curl, CURLOPT_URL, URL);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		if (message!= NULL){
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long ) strlen(message));
		}
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, operation); /* PUT, POST, ... */
// 		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		if(filenamepath!= NULL){
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_global_init(CURL_GLOBAL_ALL);
			curl_formadd(&formpost,
				&lastptr,
				CURLFORM_COPYNAME, "UploadJSON",
				CURLFORM_FILE, filenamepath,
	// 			CURLFORM_CONTENTTYPE, "application/octet-stream",
				CURLFORM_END);
		}	
		if(filenamepath!= NULL){
	// 		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);kaka
			curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		}
	}else{
		curl = prepare_publish(URL, message, NULL, operation, token);
	}
	if(curl == NULL) {
		free(data.data); data.data=NULL;
		free(data.headercode); data.headercode=NULL;
		free(rescode.data); rescode.data=NULL;
		free(rescode.headercode); rescode.headercode=NULL;
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
// 	printf("\n data is %s\n", data.data);
	free(rescode.data); rescode.data=NULL;
	//curl_slist_free_all(headers);/* free the list again */

	if(filenamepath!=NULL)
		curl_formfree(formpost);
	curl_easy_cleanup(curl);
// 	curl_global_cleanup();kkj
	if(response_code != CURLE_OK) {
		const char *error_msg = curl_easy_strerror(response_code);
		log_error("ERROR query_message_json %s", error_msg);
		free(data.data); data.data=NULL;
		free(data.headercode); data.headercode=NULL;
		free(rescode.data); rescode.data=NULL;
		free(rescode.headercode); rescode.headercode=NULL;
		return FAILED;
	}
	if (strcmp(rescode.headercode, "401") == 0) {
// 		const char *error_msg = curl_easy_strerror(response_code);
		log_error("ERROR unauthorized request, %s", data.data);
		log_error("token is %s", token);
		free(data.data); data.data=NULL;
		free(data.headercode); data.headercode=NULL;
		free(rescode.data); rescode.data=NULL;
		free(rescode.headercode); rescode.headercode=NULL;
		return FAILED;
	}

	if(data.data[0]=='\0') {
		printf("ERROR!! : query with %s failed.\n", URL);
		free(data.data); data.data=NULL;
		free(data.headercode); data.headercode=NULL;
		free(rescode.data); rescode.data=NULL;
		free(rescode.headercode); rescode.headercode=NULL;
		return FAILED;
	}
	if(response->data !=NULL) free(response->data);
	if(response->headercode !=NULL) free(response->headercode);
	response->data=data.data;
	response->headercode =rescode.headercode;

	free(data.headercode); data.headercode=NULL;
	free(rescode.data); rescode.data=NULL;
	if(response->data == NULL)
		return FAILED;
	return SUCCESS;
}

/**
*@return the the monitoring conf
*/
char* query_mf_config(char *server, char *platform_id, char *token) {
	char *URL = NULL;
	struct url_data response;
	response.size=0;
	response.data=NULL;
	response.headercode=NULL;
	char *msg = NULL;

	URL=concat_and_free(&URL, server);
	URL=concat_and_free(&URL, "/query_device_mf_config?pretty=true&device=\"");
	URL=concat_and_free(&URL, platform_id);
	URL=concat_and_free(&URL, "\"");

// 	printf("******* new_create_new_experiment ******\n"); 
	char operation[]="GET";
	if(query_message_json(URL, msg, NULL, &response, operation, token)==FAILED){
		printf("ERROR: searching for DEFAULT mf_config for device %s\n", platform_id);
		if(msg!=NULL) {free(msg); msg=NULL;}
		if(URL!=NULL) {free(URL); URL=NULL;}
		if(response.data!=NULL) {free(response.data); response.data=NULL;}
		if(response.headercode!=NULL) {free(response.headercode); response.headercode=NULL;}
		return NULL;
	}
	
	if(msg!=NULL) {free(msg); msg=NULL;}
	if(URL!=NULL) {free(URL); URL=NULL;}
	if(response.headercode!=NULL) {free(response.headercode); response.headercode=NULL;}
	if(response.data==NULL){
		printf("ERROR: on response.data when searching for DEFAULT mf_config for device %s\n", platform_id);
		return NULL;
	}
	if(response.data[0] == '\0') {
		printf("ERROR: Cannot find DEFAULT mf_config for device %s\n", platform_id);
		return NULL;
	}
	return response.data; // it contains the mf_config;
}

/* create new experiment for specific application
read back the generated experiment_id, after send the msg
return 1 on success; otherwise return 0 */
// int create_new_experiment(char *URL, char *message, char *experiment_id, const char *token) {
// 	//You should use query_message_json instead of this function create_new_experiment !!
// 	if(check_URL(URL)!=SUCCESS || check_message(message)!=SUCCESS)
// 		return FAILED;
// 	CURL *curl = prepare_publish(URL, message, NULL, "POST", token);
// 	if(curl == NULL)
// 		return FAILED;
// 	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_stream_data);
// 	curl_easy_setopt(curl, CURLOPT_WRITEDATA, experiment_id);
// 	CURLcode response = curl_easy_perform(curl);
// 	if(response != CURLE_OK) {
// 		const char *error_msg = curl_easy_strerror(response);
// 		log_error("create_new_experiment %s", error_msg);
// 		return FAILED;
// 	}
//	//curl_slist_free_all(headers);/* free the list again */
// 	curl_easy_cleanup(curl);
// 	return SUCCESS;
// }

/** Check if the url is set
* @return 0 on success; otherwise returns 1 */
int check_URL(const char *URL) {
	if(URL == NULL || *URL == '\0') {
		const char *error_msg = "URL not set.";
		log_error("publish(char *, Message) %s", error_msg);
// 		printf("check_URL(char *, Message) %s\n", error_msg);
		return FAILED;
	}
	return SUCCESS;
}

/** Check if the message is set
* @return 1 on success; otherwise return 0 */
int check_message(const char *message) {
	if(message == NULL || *message == '\0') {
		const char *error_msg = "message not set.";
		log_error("publish(char *, Message) %s", error_msg);
		return FAILED;
	}
	return SUCCESS;
}

/* Initialize libcurl; set headers format */
void init_curl(const char *token) {
	if(headers != NULL )
		return;
	// struct curl_slist *headers= NULL;
	curl_global_init(CURL_GLOBAL_ALL);
	if(token !=NULL){
		if(token[0]!='0'){
			const char authorization_header[]="Authorization: OAuth ";
			unsigned int size=strlen(authorization_header) + strlen(token)+1;
			char *newheader = (char *) malloc(size);
			if(newheader==NULL) {
				printf("Failed to allocate memory.\n");
				exit(1);
			}
			newheader[0]='0';
			strcpy(newheader,authorization_header);
			strcat(newheader,token);
			headers = curl_slist_append(headers, newheader);
			free(newheader);
		}
	}
// 	headers = curl_slist_append(headers, string("X-Auth-Token: " + token).c_str());
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charsets: utf-8");
}
// DESCRIPTION: This function releases resources acquired by curl_global_init.
void close_curl(void) {
// 	printf(" *****************  CLOSE CURL ***************\n");
	curl_global_cleanup( );
}


/** Prepare for using libcurl with message */
CURL *prepare_publish(char *URL, char *message, FILE *send_fp, char *operation, const char *token) {
// struct curl_httppost *formpost = NULL;
// struct curl_httppost *lastptr = NULL;
// 		if(send_fp!= NULL){
// 		 curl_global_init(CURL_GLOBAL_ALL);
		
// curl_mime *form = NULL;
// curl_mimepart *field = NULL;
// 
// form = curl_mime_init(curl);
// 
// /* Fill in the file upload field */ 
// field = curl_mime_addpart(form);
// curl_mime_name(field, "UploadJSON");
// curl_mime_filedata(field, "/home/jmontana/phantom_mf/Monitoring_client/my-json-parser/exec_stats.json");
// 
// //     /* Fill in the filename field */ 
// //     field = curl_mime_addpart(form);
// //     curl_mime_name(field, "filename");
// //     curl_mime_data(field, "postit2.c", CURL_ZERO_TERMINATED);
//  
//     /* Fill in the submit field too, even if this is rarely needed */ 
// //     field = curl_mime_addpart(form);
// //     curl_mime_name(field, "submit");
// //     curl_mime_data(field, "send", CURL_ZERO_TERMINATED);

// curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

	//  ******************************************	

// curl_formadd(&formpost,
//              &lastptr,
//              CURLFORM_COPYNAME, "UploadJSON",
//              CURLFORM_FILE, "/home/jmontana/phantom_mf/Monitoring_client/my-json-parser/exec_stats.json",
//              CURLFORM_END);
//other files should be next
// curl_formadd(&formpost,
//              &lastptr,
//              CURLFORM_COPYNAME, "UploadJSON",
//              CURLFORM_COPYCONTENTS, "exec_stats.json",
//              CURLFORM_END);


//  ******************************************
// 		/* to get the file size */
// 		printf("set the file for the curl !!\n");
// 		if(fstat(fileno(send_fp), &file_info) != 0)
// 			return NULL; /* can't continue */
// 
// 		/* tell it to "upload" to the URL */
// 		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
// 
// 		/* set where to read from */
// 		curl_easy_setopt(curl, CURLOPT_READDATA, send_fp);//opened for read operation
// 
// 		/* and give the size of the upload (optional) */
// 		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
// 	}
	
// 	struct stat file_info;
	init_curl(token);//this defined the headers
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);//not using signals to notify timeouts on requests and seems not to work fine with multi-threading
	curl_easy_setopt(curl, CURLOPT_URL, URL);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	if (message!= NULL){
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long ) strlen(message));
	}
// 		if(send_fp!= NULL){
// 			curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
// 		}
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, operation); /* PUT, POST, ... */

	#ifdef DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	#endif
	return curl;
}

/** Prepare for using libcurl without message
* Leave the token as NULL or empty string if not using tokens*/
CURL *prepare_query(char* URL, char *operation, const char *token) {
	CURL *curl = curl_easy_init();
	if(curl) {
		init_curl(token);//this defined the headers
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);//not using signals to notify timeouts on requests and seems not to work fine with multi-threading
		curl_easy_setopt(curl, CURLOPT_URL, URL);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, operation); /* GET, PUT... */
		#ifdef DEBUG
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		#endif
		//  curl_slist_free_all(headers);
	}
	return curl;
}
