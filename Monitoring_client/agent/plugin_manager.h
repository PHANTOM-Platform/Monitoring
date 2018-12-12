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

#ifndef PLUGIN_MANAGER_H_
#define PLUGIN_MANAGER_H_

#include <excess_concurrent_queue.h>

/**
 * @brief Entry function of a plug-in
 */
typedef char* (*PluginHook)();

/**
 * @brief definition of plugin manager struct
 */
typedef struct PluginManager_t {
	EXCESS_concurrent_queue_t hook_queue;
} PluginManager;

/**
 * @brief definition of plugin hook
 */
typedef struct PluginHookType_t {
	PluginHook hook;
	const char *name;
} PluginHookType;

/**
 * @brief Initializing the plug-in manager
 */
PluginManager* PluginManager_new();

/**
 * @brief Functions to deallocate the memory used at run-time
 */
void PluginManager_free(PluginManager *pm);

/**
 * @brief Registers hooks, i.e. entry functions, for each plug-in
 */
void PluginManager_register_hook(PluginManager *pm, const char *name, PluginHook hook);

/**
 * @brief Return the hook function associated with the given plug-in
 *
 * It should be noted that hooks are stored in a FIFO queue.
 *
 * @returns a hook function
 */
PluginHook PluginManager_get_hook(PluginManager *pm);

#endif /* PLUGIN_MANAGER_H_ */
