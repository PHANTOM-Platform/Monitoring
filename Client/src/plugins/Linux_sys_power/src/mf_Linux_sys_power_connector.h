/*
 * Copyright (C) 2015-2017 University of Stuttgart
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

#ifndef _LINUX_SYS_POWER_CONNECTOR_H
#define _LINUX_SYS_POWER_CONNECTOR_H

#include <plugin_utils.h>

/** @brief Initializes the Linux system power plugin
 *
 *  @return 1 on success; 0 otherwise.
 */
int mf_Linux_sys_power_init(Plugin_metrics *data, char **events, size_t num_events);


/** @brief Samples all possible events and stores data into the Plugin_metrics
 *
 *  @return 1 on success; 0 otherwise.
 */
int mf_Linux_sys_power_sample(Plugin_metrics *data);


/** @brief Formats the sampling data into a json string
 *
 *  json string contains: plugin name, timestamps, metrics_name and metrics_value
 *
 */
void mf_Linux_sys_power_to_json(Plugin_metrics *data, char *json);


#endif /* _LINUX_RESOURCES_CONNECTOR_H */