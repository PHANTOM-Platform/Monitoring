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

#include <stdlib.h>
#include <stdio.h>
#include <excess_concurrent_queue.h>
#include "plugin_manager.h"
#include "mf_debug.h"		//functions like log_error(), log_info()...

/* Creat a new plugin manager, which contains a hook queue */
PluginManager* PluginManager_new() {
	PluginManager *pm = malloc(sizeof(PluginManager));
	pm->hook_queue = ECQ_create(0);
	return pm;
}

/* Clean-up a plugin manager */
void PluginManager_free(PluginManager *pm) {
	ECQ_free(pm->hook_queue);
	free(pm);
}

/* Register a plugin hook to the plugin manager 
* push the hook to the hook queue */
void PluginManager_register_hook(PluginManager *pm, const char *name, PluginHook hook) {
	PluginHookType *hookType = malloc(sizeof(PluginHookType));
	hookType->hook = hook;
	hookType->name = name;

	log_info("register hookType->name %s\n", hookType->name);	
	
	EXCESS_concurrent_queue_handle_t hook_queue_handle;
	hook_queue_handle = ECQ_get_handle(pm->hook_queue);
	ECQ_enqueue(hook_queue_handle, (void *)hookType);
	ECQ_free_handle(hook_queue_handle);
}

/* Pop one hook from the hook queue */
PluginHook PluginManager_get_hook(PluginManager *pm) {
	PluginHook funcPtr = NULL;
	void *retPtr;
	
	EXCESS_concurrent_queue_handle_t hook_queue_handle;
	hook_queue_handle =ECQ_get_handle(pm->hook_queue);

	if(ECQ_try_dequeue(hook_queue_handle, &retPtr)) {
		PluginHookType *typePtr;
		typePtr = (struct PluginHookType_t *) retPtr;
		funcPtr = *(typePtr->hook);
		log_info("Using plugin %s\n", typePtr->name);	
		printf("Using plugin %s\n", typePtr->name);	
	}

	ECQ_free_handle(hook_queue_handle);
	return funcPtr;
}
