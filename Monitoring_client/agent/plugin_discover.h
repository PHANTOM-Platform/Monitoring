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

#ifndef PLUGIN_DISCOVER_H_
#define PLUGIN_DISCOVER_H_

#include "plugin_manager.h"

/**
 * @brief Number of plug-ins found at run-time
 */
extern int pluginCount;

/**
 * @brief Name of a given plug-in
 */
extern char* plugins_name[256];

/**
 * @brief Discovers available plug-ins, and registers them to the plugin manager
 */
void* discover_plugins(const char *dirname, PluginManager *pm);

/**
 * @brief Clean-up plug-ins after execution
 */
void cleanup_plugins(void*);

#endif /* PLUGIN_DISCOVER_H_ */
