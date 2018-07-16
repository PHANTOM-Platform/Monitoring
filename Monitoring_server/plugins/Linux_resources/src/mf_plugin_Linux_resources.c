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
#include <stdio.h>
#include <stdlib.h> /* malloc etc */
#include <string.h>
#include <time.h>
#include <plugin_manager.h> /* mf_plugin_xxx_hook */
#include <mf_parser.h> /* mfp_data */
#include <mf_debug.h>
#include <plugin_utils.h> /* Plugin_metrics */
#include "mf_Linux_resources_connector.h"

/*******************************************************************************
 * Variable Declarations
 ******************************************************************************/
mfp_data *conf_data = NULL;
Plugin_metrics *monitoring_data = NULL;
int is_initialized = 0;

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
char* mf_plugin_Linux_resources_hook();

/*******************************************************************************
 * Functions implementation
 ******************************************************************************/
/* Initialize the plugin; 
   register the plugin hook to the plugin manager 
   @return 1 on success; 0 otherwise */
extern int
init_mf_plugin_Linux_resources(PluginManager *pm)
{
    /*
     * get the turned on metrics from the configuration file
     */
    conf_data =  malloc(sizeof(mfp_data));
    mfp_get_data_filtered_by_value("mf_plugin_Linux_resources", conf_data, "on");

    /*
     * initialize the monitoring data
     */
    monitoring_data = malloc(sizeof(Plugin_metrics));
    int ret = mf_Linux_resources_init(monitoring_data, conf_data->keys, conf_data->size);
    if(ret == 0) {
        char plugin_name[] = "Linux_resources";
        log_error("Plugin %s init function failed.\n", plugin_name);
        return ret;
    }
    /*
     * if init succeed; register the plugin hook to the plugin manager
     */
    PluginManager_register_hook(pm, "mf_plugin_Linux_resources", mf_plugin_Linux_resources_hook);
    is_initialized = 1;
    return ret;
}

/* the hook function, sample the metrics and convert to a json-formatted string */
char*
mf_plugin_Linux_resources_hook()
{
    if (is_initialized) {
        /*
         * sampling 
         */
        mf_Linux_resources_sample(monitoring_data);

        /*
         * Prepares a json string, including current timestamp, name of the plugin,
         * and required metrics.
         */
        char *json = calloc(JSON_MAX_LEN, sizeof(char));
        mf_Linux_resources_to_json(monitoring_data, json);

        return json;
    } else {
        return NULL;
    }
}
