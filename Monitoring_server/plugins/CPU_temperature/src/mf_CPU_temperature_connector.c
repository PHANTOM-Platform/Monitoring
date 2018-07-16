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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sensors.h>
#include "mf_CPU_temperature_connector.h"

#define SUCCESS 1
#define FAILURE 0

/*******************************************************************************
 * Variable Declarations
 ******************************************************************************/
typedef struct requested_features_t {
    const sensors_chip_name **chip;
    const sensors_subfeature **subfeature;
    Plugin_metrics *data;
} requested_features;

requested_features *features_list = NULL;

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre));
}

/*******************************************************************************
 * Functions implementation
 ******************************************************************************/
/** @brief Initializes the CPU_temperature plugin
 *
 *  Load libsensors library; prepare features_list for sampling
 *  @return 1 on success; 0 otherwise.
 */
int mf_CPU_temperature_init(Plugin_metrics *data, char **events, size_t num_events)
{
	int retval = sensors_init(NULL);
    if (retval != 0) {
        fprintf(stderr, "ERROR: Couldn't initiate lm-sensors.\n");
        return FAILURE;
    }

    /* initialize features list */
    features_list = malloc(sizeof(requested_features));
    features_list->chip = malloc(MAX_EVENTS_NUMBER * sizeof(sensors_chip_name *));
    features_list->subfeature = malloc(MAX_EVENTS_NUMBER * sizeof(sensors_subfeature *));
    features_list->data = data;

    int event_i = 0;
    const sensors_chip_name *chip;
    int chip_num = 0;
    while ((chip = sensors_get_detected_chips(NULL, &chip_num)) != NULL) {
        if (strcmp(chip->prefix, "coretemp") != 0) {
            continue;
        }

        /* get chipnum */
        char chipnum[2] = {'\0'};
        const sensors_feature *feature_tmp, *feature;
        int feature_num = 0;
        while ((feature_tmp = sensors_get_features(chip, &feature_num)) != NULL) {
            if (feature_tmp->type != SENSORS_FEATURE_TEMP) {
                continue;
            }
            char *label = sensors_get_label(chip, feature_tmp);
            if (prefix("Physical id", label) != 0) {
                continue;
            }
            strncpy(chipnum, label+12, strlen(label)-12);
        }

        /* create sensors features according to given events */
        feature_num = 0;
        while ((feature = sensors_get_features(chip, &feature_num)) != NULL) {
            const sensors_subfeature *subfeature;
            int subfeature_num = 0;

            if (feature->type != SENSORS_FEATURE_TEMP) {
                continue;
            }

            while ((subfeature = sensors_get_all_subfeatures (chip, feature, &subfeature_num)) != NULL) {
                if (subfeature->type != SENSORS_SUBFEATURE_TEMP_INPUT) {
                    continue;
                }

                /* get corenum */
                char *label = sensors_get_label(chip, feature);
                if (prefix("Core", label) != 0) {
                    continue;
                }
                char corenum[3] = {'\0'};
                strncpy(corenum, label+5, strlen(label)-5);

                /* get my_label with acquired chipnum and corenum */
                char my_label[16] = {'\0'};
                sprintf(my_label, "CPU%s:core%s", chipnum, corenum);
                
                /* use flag to mark if my_label can be found in the given events */
                int i, flag = 0;
                for (i = 0; i < num_events; i++) {
                    if(strcmp(events[i], my_label) == 0) {
                        flag = 1;
                        break;
                    }
                }
                if(flag == 0) {
                    continue;
                }
                else {
                	features_list->chip[event_i] = chip;
                	features_list->subfeature[event_i] = subfeature;
                	features_list->data->events[event_i] = malloc(MAX_EVENTS_LEN * sizeof(char));
                	strcpy(data->events[event_i], my_label);
                	event_i++;
                	break;
                }
            }
        }
    }
    if(event_i == 0) {
    	fprintf(stderr, "ERROR: No events can be measured.\n");
    	return FAILURE;
    }
    else {
    	features_list->data->num_events = event_i;
    }
	return SUCCESS;
}

/** @brief Samples all possible events and stores data into the Plugin_metrics
 *
 *  @return 1 on success; 0 otherwise.
 */
int mf_CPU_temperature_sample(Plugin_metrics *data)
{
	int i;
	double value;
	for (i = 0; i < data->num_events; i++) {
		sensors_get_value(features_list->chip[i], features_list->subfeature[i]->number, &value);
		data->values[i] = (float) value;
	}

	return SUCCESS;
}

/** @brief Formats the sampling data into a json string
 *
 *  json string contains: plugin name, timestamps, metrics_name and metrics_value
 *
 */
void mf_CPU_temperature_to_json(Plugin_metrics *data, char *json)
{
	struct timespec timestamp;
    char tmp[128] = {'\0'};
    int i;
    /*
     * prepares the json string, including current timestamp, and name of the plugin
     */
    sprintf(json, "\"type\":\"CPU_temperature\"");
    clock_gettime(CLOCK_REALTIME, &timestamp);
    double ts = timestamp.tv_sec * 1.0e3 + (double)(timestamp.tv_nsec / 1.0e6); // in millisecond
    sprintf(tmp, ",\"local_timestamp\":\"%.1f\"", ts);
    strcat(json, tmp);

    /*
     * prepend metrics names and values into the json string
     */
	for (i = 0; i < data->num_events; i++) {
		sprintf(tmp, ",\"%s\":%.3f", data->events[i], data->values[i]);
		strcat(json, tmp);
	}
}

/** @brief Stops the plugin
 *
 *  This methods stops papi counters gracefully;
 *
 */
void mf_CPU_temperature_shutdown()
{
	free(features_list->chip);
	free(features_list->subfeature);
	free(features_list);
	sensors_cleanup();
}
