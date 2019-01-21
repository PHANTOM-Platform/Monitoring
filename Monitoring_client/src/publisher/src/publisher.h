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

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

/** definitions for new_query_json */
struct url_data {
	size_t size;
	char* headercode;
	char* data;
};

/** if the token is NULL or empty string, will procedd as not token provided or required*/
int new_query_json(char *URL, struct url_data *response, char *operation, const char *token);

// int query_json(char *URL, char *response);

/**
 * @brief Sends the data defined in message to the given URL via cURL.
 *
 * @return 1 if successful; 0 otherwise
 */
int publish_json(char *URL, char *message, const char *token);

int publish_file(char *URL, char *static_string, char *filename, const char *token);

int query_message_json(char *URL, char *message,const char *filenamepath, struct url_data *response, char *operation, const char *token);

void close_curl(void);

// int create_new_experiment(char *URL, char *message, char *experiment_id, const char *token);
#endif
