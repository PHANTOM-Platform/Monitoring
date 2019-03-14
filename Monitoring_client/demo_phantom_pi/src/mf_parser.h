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

/**
* @brief Parses a given file following the ini configuration format.
*
* The Parser wraps the access to the configuration file. The format of the
* configuration follows the INI file format.
*
* @see http://en.wikipedia.org/wiki/INI_file
*
* @author Dennis Hoppe <hoppe@hrls.de>
* @author J.M.Montanana <montanana@hrls.de>
*/
#ifndef MF_PARSER_H_
#define MF_PARSER_H_

typedef struct {
	char* keys[256];
	char* values[256];
	int size;
} mfp_data;

/**
* @brief Parses a given file.
*
* It should be noted that each time this function is called, the internal data
* structure is cleared in advanced. This might currently result in some
* inconsistencies while accessing the data in a concurrent setting.
*
* @return 1 if successful; 0 otherwise.
*/
int mfp_parse(const char* filename);

/**
* @brief Returns a stored value for the given section and key.
*
* @return the value stored for the index <section, key>
*/
void mfp_get_value(const char* section, const char* key, char *ret_val);

/**
* @brief Sets or overwrites the value for a given section and key.
*
*/
void mfp_set_value(const char* section, const char* key, const char* value);

/**
* @brief Returns the entire data stored for a given section.
*
*/
void mfp_get_data(const char* section, mfp_data* data);

/**
* @brief Filters the data based on the given filter.
*
* Example: filter = "on" will also retain those keys in the data set which
* value equals the filter value.
*/
void mfp_get_data_filtered_by_value(const char* section, mfp_data* data, const char* filter_by_value);

/**
* @brief Frees the allocated memory for the configuration data
*
*/
void mfp_data_free(mfp_data* data);

/**
* @brief Clears the memory for apr pool and tears down the apr internal data structures
*
*/
void mfp_parse_clean();




#define SUCCESS 0
#define FAILED 1


	
struct json_object{//level 3, not count, keep labels and values
	char *label_o;
	char *value_o;
};

struct json_mf_config_field{//level 2, count and keep only labels
	int count_o;
	char *label_f;
	struct json_object **object;
};

struct json_mf_config{//level 1, just only count
	int count_f;
	struct json_mf_config_field **field;
};

char *query_for_plugin_parsed_json(const unsigned int loaded_conf, char *plugin_id, struct json_mf_config **mf_config,const unsigned int total_loaded_mf_configs);

int query_for_platform_parsed_json(const char *platform_id, struct json_mf_config **mf_config,const unsigned int total_loaded_mf_configs);

int print_counters_from_json(char *html);
int print_counters_from_parsed_json(struct json_mf_config **mf_config,const unsigned int total_loaded_mf_configs);
int print_json(char *html);
int print_parsed_json(struct json_mf_config **mf_config,const unsigned int total_loaded_mf_configs);
int parse_mf_config_json(char *html, struct json_mf_config ***in_mf_config,unsigned int *total_loaded_mf_configs);
char* query_mf_config(const char *server, const char *platform_id, const char *token);
#endif
