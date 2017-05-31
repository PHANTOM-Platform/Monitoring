/*
 * Copyright (C) 2014-2015 University of Stuttgart
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

#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include "mf_debug.h"
#include "publisher.h"

#define SUCCESS 1
#define FAILED  0
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
CURL *prepare_publish(char *URL, char *message);
CURL *prepare_query(char* URL);
static size_t get_stream_data(void *buffer, size_t size, size_t nmemb, char *stream);

#ifdef NDEBUG
static size_t write_non_data(void *buffer, size_t size, size_t nmemb, void *userp);
#endif

/* send query to the given URL, read back the response string
   return 1 on success; otherwise return 0 */
int query_json(char *URL, char *response_str)
{
    if (!check_URL(URL)) {
        return FAILED;
    }

    CURL *curl = prepare_query(URL);
    if (curl == NULL) {
        return FAILED;
    }
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_stream_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_str);
    CURLcode response = curl_easy_perform(curl);

    if (response != CURLE_OK) {
        const char *error_msg = curl_easy_strerror(response);
        log_error("create_new_experiment %s", error_msg);
        return FAILED;
    }

    curl_easy_cleanup(curl);

    if(response_str == NULL) {
        return FAILED;
    }
    return SUCCESS;
}

/* json-formatted data publish using libcurl
   return 1 on success; otherwise return 0 */
int publish_json(char *URL, char *message)
{
    if (!check_URL(URL) || !check_message(message)) {
        return FAILED;
    }
    CURL *curl = prepare_publish(URL, message);
    if (curl == NULL) {
        return FAILED;
    }

    #ifdef NDEBUG
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_non_data);
    #endif

    CURLcode response = curl_easy_perform(curl);

    if (response != CURLE_OK) {
        const char *error_msg = curl_easy_strerror(response);
        log_error("publish(char *, Message) %s", error_msg);
        return FAILED;
    }

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
    
    /* int curl with meaningless message */
    CURL *curl = prepare_publish(URL, message);
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
                CURLcode response = curl_easy_perform(curl);

                if (response != CURLE_OK) {
                    const char *error_msg = curl_easy_strerror(response);
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
        CURLcode response = curl_easy_perform(curl);

        if (response != CURLE_OK) {
            const char *error_msg = curl_easy_strerror(response);
            log_error("publish(char *, Message) %s", error_msg);
            return FAILED;
        }
    }
    /* clean the curl handle */
    curl_easy_cleanup(curl);
    return SUCCESS;
}

/* create new experiment for specific application
   read back the generated experiment_id, after send the msg
   return 1 on success; otherwise return 0 */
int create_new_experiment(char *URL, char *message, char *experiment_id)
{
    if (!check_URL(URL) || !check_message(message)) {
        return FAILED;
    }
    CURL *curl = prepare_publish(URL, message);
    if (curl == NULL) {
        return FAILED;
    }
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_stream_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, experiment_id);
    CURLcode response = curl_easy_perform(curl);

    if (response != CURLE_OK) {
        const char *error_msg = curl_easy_strerror(response);
        log_error("create_new_experiment %s", error_msg);
        return FAILED;
    }

    curl_easy_cleanup(curl);
    return SUCCESS;
}

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
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charsets: utf-8");
}

/* Prepare for using libcurl to write */
CURL *prepare_publish(char *URL, char *message)
{
    init_curl();
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long ) strlen(message));
    
    #ifdef DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    #endif

    return curl;
}

/* Prepare for using libcurl to read */
CURL *prepare_query(char* URL)
{
    init_curl();
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
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

/* Callback function to get stream data during writing */
static size_t get_stream_data(void *buffer, size_t size, size_t nmemb, char *stream) 
{
    size_t total = size * nmemb;
    /* re-allocate string length */
    stream = realloc(stream, total + 1);
    if(stream == NULL) {
        return 0;
    }

    memcpy(stream, buffer, total);
    stream[total] = '\0';

    return total;
}