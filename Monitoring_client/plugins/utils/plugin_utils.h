/*
 * Copyright (C) 2018 University of Stuttgart
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
#ifndef _PLUGIN_UTILS_H
#define _PLUGIN_UTILS_H

#define MAX_EVENTS_NUMBER 32
#define MAX_EVENTS_LEN 32
#define JSON_MAX_LEN 1024

/** @brief data structure to store plugin metrics
 *
 * The data structure holds the metric names including the correspond
 * measured values. Moreover, the number of events measured is stored.
 */
typedef struct Plugin_metrics_t
{
    char *events[MAX_EVENTS_NUMBER];
    float values[MAX_EVENTS_NUMBER];
    int num_events;
} Plugin_metrics;

#endif /* _PLUGIN_UTILS_H */
