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

#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "plugin_discover.h"
#include "mf_debug.h"		//functions like log_error(), log_info()...
#include "mf_parser.h"	//mfp_get_value()

/*******************************************************************************
* Variable Declarations
******************************************************************************/
/* List of pointers of plugin handles */
typedef struct PluginHandleList_t {
	void* handle;
	struct PluginHandleList_t* next;
} PluginHandleList;

/* First node of the list*/
typedef struct PluginDiscoveryState_t {
	PluginHandleList* handle_list;
} PluginDiscoveryState;

/* Plugin init function */
typedef int (*PluginInitFunc)(PluginManager *pm);

int pluginCount = 0;

char* plugins_name[256];

/*******************************************************************************
* Forward Declarations
******************************************************************************/
/* Load a plugin by calling the init function of a plugin */
void* load_plugin(char *name, char *fullpath, PluginManager *pm);

/* parse mf_config.ini
* if one plugin is switched on, call load_plugin function to load the plugin,
* store all the loaded plugin handlers to a list */
void* discover_plugins(const char *dirname, PluginManager *pm) {
	DIR* dir = opendir(dirname);
	struct dirent *direntry;
	if (!dir) {
		log_error("Unable to open directory %s!\n", dirname);
		return NULL;
	}
	PluginDiscoveryState *plugins_state = malloc(sizeof(*plugins_state));
	plugins_state->handle_list = NULL;
	while ((direntry = readdir(dir))) {
		/*get the name of the plugin*/
		char *last_slash = strrchr(direntry->d_name, '/');
		char *name_start = last_slash ? last_slash + 1 : direntry->d_name;
		char *last_dot = strrchr(direntry->d_name, '.');

		if (!last_dot || strcmp(last_dot, ".so"))
			continue;
		char *name = calloc(last_dot - name_start + 1, sizeof(char));
		strncpy(name, name_start, last_dot - name_start);

		if (!name)
			continue;
		/* do not consider plug-ins that are switched off */
		char value[20] = {'\0'};
		mfp_get_value("plugins", name, value);
		if (strcmp(value, "off") == 0) {
			free(name);
			continue;
		}
		char *fullpath = malloc(200 * sizeof(char));

		strcpy(fullpath, dirname);
		strcat(fullpath, "/");
		strcat(fullpath, direntry->d_name);
		plugins_name[pluginCount] = malloc(sizeof(char) * 256);
		strcpy(plugins_name[pluginCount], name);
		void *handle = load_plugin(name, fullpath, pm);
		if (handle) {
			PluginHandleList *handle_node = malloc(sizeof(*handle_node));
			handle_node->handle = handle;
			handle_node->next = plugins_state->handle_list;
			plugins_state->handle_list = handle_node;
			pluginCount++;
		}
		free(name);
		free(fullpath);
	}
	printf(" pluginCount is %i\n",pluginCount);
	closedir(dir);
	if (plugins_state->handle_list)
		return (void*) plugins_state;
	else {
		free(plugins_state);
		return NULL;
	}
	return 0;
}

/** Clean-up plug-ins after execution */
void cleanup_plugins(void* vpds) {
	if(vpds != NULL) {
		PluginDiscoveryState *pds = (PluginDiscoveryState*) vpds;
		PluginHandleList *node = pds->handle_list;
		while (node) {
			PluginHandleList *next = node->next;
			dlclose(node->handle);
			free(node);
			node = next;
		}
		free(pds);
	}
}

/* Load a plugin by calling the init function of a plugin,
when successfully, the plugin hook function is registered by the 
PluginManager */
void* load_plugin(char *name, char *fullpath, PluginManager *pm) {
	char* slashed_path = strdup(fullpath);
	void *libhandle = dlopen(slashed_path, RTLD_NOW);
	if (!libhandle) {
		log_error("Unable to load library %s\n", dlerror());
		return NULL;
	}
	char *init_func_name = malloc((strlen("init_") + strlen(name)) * sizeof(char) + 1);
	strcpy(init_func_name, "init_");
	strcat(init_func_name, name);
	void *ptr = dlsym(libhandle, init_func_name);
	free(init_func_name);
	if (!ptr) {
		log_error("Unable to load init function %s\n", dlerror());
		return NULL;
	}
	PluginInitFunc init_func = (PluginInitFunc) (intptr_t) ptr;
	int rc = init_func(pm);
	if (rc <= 0) {
		log_error("Plugin init function failed for %s\n", strerror(rc));
		dlclose(libhandle);
		return NULL;
	}
	printf(" loaded plugin %s %s\n",name,fullpath);
	free(slashed_path);
	return libhandle;
}
