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

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

int query_json(char *URL, char *response);

/**
 * @brief Sends the data defined in message to the given URL via cURL.
 *
 * @return 1 if successful; 0 otherwise
 */
int publish_json(char *URL, char *message);

int publish_file(char *URL, char *static_string, char *filename);

int create_new_experiment(char *URL, char *message, char *experiment_id);
#endif