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

#if __GNUC__
#if __x86_64__ || __ppc64__
#else
#define _LARGEFILE64_SOURCE
#endif
#endif

#include <apr.h>
#include <apr_hash.h>
#include <apr_strings.h>
#include <apr_general.h>
#include <stdlib.h>

#include "../libs/ini/ini.h"
#include "mf_debug.h"
#include "mf_parser.h"

#define MAKE_DUP(s) apr_pstrdup(mp, s)

static apr_pool_t *mp;
static apr_hash_t *ht_config;
static int ht_initialized = 0;

static int handle_parser(void*, const char*, const char*, const char*);

/* Parses a given file */
int
mfp_parse(const char* filename)
{
    /* Initialize a hash table */
    apr_initialize();
    apr_pool_create(&mp, NULL);
    ht_config = apr_hash_make(mp);
    ht_initialized = 1;

    /* Parse a INI file and write the name: value into a harsh table created */
    int error = ini_parse(filename, handle_parser, ht_config);
    if (error < 0) {
        log_error("mfp_parse(const char*) Can't load %s", filename);
        return 0;
    }

    return 1;
}

/* Sets or overwrites the value for a given section and key */
static int
handle_parser(void* user, const char* section, const char* name, const char* value)
{
    mfp_set_value(section, name, value);
    return 1;
}

/* Sets or overwrites the value for a given section and key */
void
mfp_set_value(const char* section, const char* key, const char* value)
{
    if (!ht_initialized) {
        return;
    }

    apr_hash_t *ht_values = apr_hash_get(ht_config, section, APR_HASH_KEY_STRING);
    if (ht_values == NULL) {
        debug("mfp_set_value(..) -- Created new hash_table for section: %s", section);
        ht_values = apr_hash_make(mp);
    }

    debug("mfp_set_value(..) -- Set new values <%s,%s>", key, value);
    apr_hash_set(ht_values, MAKE_DUP(key), APR_HASH_KEY_STRING, MAKE_DUP(value));
    apr_hash_set(ht_config, MAKE_DUP(section), APR_HASH_KEY_STRING, ht_values);
}

/* Returns a stored value for the given section and key */
void
mfp_get_value(const char* section, const char* key, char *ret_val)
{
    if (!ht_initialized) {
        return;
    }

    apr_hash_t *ht_values = apr_hash_get(ht_config, section, APR_HASH_KEY_STRING);
    if (ht_values == NULL) {
        log_error("mfp_get_value(const char*, const char*) Key does not exist: <%s:%s>", section, key);
        return;
    }
    const char *ht_key = apr_hash_get(ht_values, key, APR_HASH_KEY_STRING);
    if (ht_key == NULL) {
        log_error("mfp_get_value(const char*, const char*) Key does not exist: <%s:%s>", section, key);
        return;
    }
    strcpy(ret_val, ht_key);
}

/* Filters the data based on the given filter */
void
mfp_get_data_filtered_by_value(const char* section, mfp_data* data, const char* filter_by_value)
{
    if (!ht_initialized) {
        return;
    }

    apr_hash_index_t *ht_index;
    apr_hash_t *ht_section;
    data->size = 0;
    ht_section = apr_hash_get(ht_config, section, APR_HASH_KEY_STRING);
    if (ht_section == NULL) {
        log_error("mfp_get_data(const char*, mfp_data*) Section does not exist: %s", section);
        return;
    }
    for (ht_index = apr_hash_first(NULL, ht_section); ht_index; ht_index = apr_hash_next(ht_index)) {
        const char *key;
        const char *value;

        apr_hash_this(ht_index, (const void**)&key, NULL, (void**)&value);

        // filter keys by value
        if (filter_by_value != NULL) {
            if (strcmp(value, filter_by_value) != 0) {
                continue;
            }
        }

        data->keys[data->size] = malloc(sizeof(char) * 256);
        strcpy(data->keys[data->size], key);
        data->values[data->size] = malloc(sizeof(char) * 256);
        strcpy(data->values[data->size], value);

        data->size++;
    }
}

/* Returns the entire data stored for a given section */
void
mfp_get_data(const char* section, mfp_data* data)
{
    mfp_get_data_filtered_by_value(section, data, NULL);
}

/* Frees the allocated memory for configuration data */
void
mfp_data_free(mfp_data* data)
{
    int i;
    for(i=0; i<data->size; i++) {
        free(data->keys[i]);
        free(data->values[i]);
    }
    free(data);
}

/* Clears the memory for apr pool and tears down the apr internal data structures */
void
mfp_parse_clean()
{
    if (!ht_initialized) {
        return;
    }
    apr_pool_destroy(mp);
    apr_terminate();
}
