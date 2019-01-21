/*
* Copyright (C) 2018 University of Stuttgart
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
* 
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
#include <malloc.h>

#include "../libs/ini/ini.h"
#include "mf_debug.h"
#include "mf_parser.h"
#include "mf_util.h"
// #include "publisher.h"

#define MAKE_DUP(s) apr_pstrdup(mp, s)

static apr_pool_t *mp;
static apr_hash_t *ht_config;
static int ht_initialized = 0;

static int handle_parser(void*, const char*, const char*, const char*);

/** Parses a given file */
int mfp_parse(const char* filename) {
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

/** Sets or overwrites the value for a given section and key */
static int handle_parser(void* user, const char* section, const char* name, const char* value) {
	mfp_set_value(section, name, value);
	return 1;
}

/** Sets or overwrites the value for a given section and key */
void mfp_set_value(const char* section, const char* key, const char* value) {
	if (!ht_initialized)
		return;
	apr_hash_t *ht_values = apr_hash_get(ht_config, section, APR_HASH_KEY_STRING);
	if (ht_values == NULL) {
		debug("mfp_set_value(..) -- Created new hash_table for section: %s", section);
		ht_values = apr_hash_make(mp);
	}
	debug("mfp_set_value(..) -- Set new values <%s,%s>", key, value);
	apr_hash_set(ht_values, MAKE_DUP(key), APR_HASH_KEY_STRING, MAKE_DUP(value));
	apr_hash_set(ht_config, MAKE_DUP(section), APR_HASH_KEY_STRING, ht_values);
}

/** @return a stored value for the given section and key */
void mfp_get_value(const char* section, const char* key, char *ret_val) {
	if (!ht_initialized)
		return;
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

/** Filters the data based on the given filter */
void mfp_get_data_filtered_by_value(const char* section, mfp_data* data, const char* filter_by_value) {
	if (!ht_initialized)
		return;
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
		data->keys[data->size] = malloc(sizeof(char) * (strlen(key)+1));
		strcpy(data->keys[data->size], key);
		data->values[data->size] = malloc(sizeof(char) * (strlen(value)+1) );
		strcpy(data->values[data->size], value);
		data->size++;
	}
}

/** Returns the entire data stored for a given section */
void mfp_get_data(const char* section, mfp_data* data) {
	mfp_get_data_filtered_by_value(section, data, NULL);
}

/** Frees the allocated memory for configuration data */
void mfp_data_free(mfp_data* data) {
	int i;
	if(data!=NULL){
		for(i=0; i<data->size; i++) {
			if (data->keys[i]!=NULL) free(data->keys[i]); 
			data->keys[i]=NULL;
			if (data->values[i]!=NULL) free(data->values[i]); 
			data->values[i]=NULL;
		}
		free(data); data=NULL;
	}
}

/** Clears the memory for apr pool and tears down the apr internal data structures */
void mfp_parse_clean(void) {
	if (!ht_initialized)
		return;
	apr_pool_destroy(mp);
	apr_terminate();
}


















/**
 *
 * @author J.M.Montanana <montanana@hrls.de>
 */
/**
 * @return a string which contains the fragment from start to end from the input string.
 */
char *return_sub_string(char *input,const int start, const int end){
	if(start>end){
		log_error("error on input parameters%s.\n","");
		exit(0);
	}
	unsigned int length=end-start;
	char *result= (char *) malloc(length+2);
	if(result==NULL) {
		log_error("Failed to allocate memory%s.\n","");
		exit(0);
	}
	unsigned int i=0;
	while(i<=length){
		result[i]=input[start+i];
		i++;
	}
	result[length+1]='\0';
	return result;
}

int count_mykeys_from_json(char *buf, const int startpos){
	int i=startpos;
	int level=0;
	int count_keys=0;
	while(i<strlen(buf)){
		if((buf[i]=='[')||(buf[i]=='{')){
			level++;
			if(level==1)
			count_keys++;
// 			printf("\n");
// 			for(j=0;j<level;j++) printf("\t");
		}else if((buf[i]==']')||(buf[i]=='}')){
			if(level==1)
				return count_keys;
// 			printf("\n");
// 			for(j=0;j<level;j++) printf("\t");
// 			printf("%i",count_keys[level-1]);
			level--;
		}else if((buf[i]==',')){
			if(level==1)
			count_keys++;
// 			printf("\n");
// 			for(j=0;j<level;j++) printf("\t");
		}
		i++;
	}
	return count_keys;
}

int parse_mf_config_json(char *html, struct json_mf_config ***in_mf_config,unsigned int *total_loaded_mf_configs){
	int num=*total_loaded_mf_configs;
	struct json_mf_config **mf_config=*in_mf_config;
// 	if html starts with "{\"hits\" :" then remove it, and the last "}" in the string.
	char hits_head[]="{\"hits\" :";
// 	int debug =false;
	int counter;
	int start,end;
	int total_fields,total_objects;
	int level=0;
	int i=0;
	if(html==NULL) return 0;
	if(strlen(html)> strlen(hits_head)){
		while ((hits_head[i]==html[i])&&(i< strlen(hits_head))){
			i++;
		}
		if(i==strlen(hits_head) ){
			for(i=0;i<strlen(html);i++){
				html[i]=html[i+strlen(hits_head)];
			}
		}
		i=strlen(html);
		while((html[i]!='}')&&(i>0)){
			i--;
		}
		if(i>0) html[i]='\0';//we remove the last character '}', and then the string finishes there.
	}
	if (html==NULL) return 0;
	for(i=0;i<strlen(html);i++){
		if((html[i]=='[')||(html[i]=='{')){
// // 			count_keys[level]+=1;
			level++;
// 			if(debug) printf("\n%s",NO_COLOUR);
// 			if(debug) for(j=0;j<level;j++) printf("\t");
		}else if((html[i]==']')||(html[i]=='}')){
// 			if(debug) printf("\n%s",NO_COLOUR);
// 			if(debug) for(j=0;j<level;j++) printf("\t");
			level--;
// 		}else if((html[i]==',')){
// // 			count_keys[level-1]+=1;
// 			if(debug) printf("\n%s",NO_COLOUR);
// 			if(debug) for(j=0;j<level;j++) printf("\t");
// 		}else if(i>0){
// 			if((html[i-1]=='\"')&&(html[i]==':'))
// 				if(debug) printf("%s",NO_COLOUR);
// 			if((html[i-1]==':'))
// 				if(debug) printf("%s",LIGHT_BLUE);
// 			if((html[i-1]=='[')||(html[i-1]=='{')||(html[i-1]==','))
// 				if(debug) printf("%s",yellow);
		}
// 		if(debug) printf("%c",html[i]);

		if((html[i]=='[')||(html[i]=='{')||(html[i]==',')){
			if(level==1){
				struct json_mf_config **new_set_level_1;
				new_set_level_1= (struct json_mf_config**) malloc((num+1)* sizeof(struct json_mf_config*));
				for(counter=0;counter<num;counter++)
					new_set_level_1[counter]=mf_config[counter];
				free(mf_config);
				mf_config=new_set_level_1;
				mf_config[num]=(struct json_mf_config*) malloc( sizeof(struct json_mf_config));
				mf_config[num]->count_f=0;
				mf_config[num]->field=NULL;
				num++;
				*total_loaded_mf_configs=num;
			}else if(level==2){
				struct json_mf_config_field **new_set_level_2;
				total_fields = mf_config[num-1]->count_f;
				new_set_level_2= (struct json_mf_config_field **) malloc((total_fields+1)* sizeof(struct json_mf_config_field*));
				for(counter=0;counter< total_fields ;counter++)
					new_set_level_2[counter]=mf_config[num-1]->field[counter];
				free(mf_config[num-1]->field);
				mf_config[num-1]->field=new_set_level_2;
				mf_config[num-1]->field[total_fields]=(struct json_mf_config_field*) malloc(sizeof(struct json_mf_config_field));
				mf_config[num-1]->field[total_fields]-> count_o =0;
				mf_config[num-1]->field[total_fields]-> object =NULL;
				//1.-searching for the begining of the label
				start=i+1;
				while((html[start]==' ')|| (html[start]=='\n')) start++;
				if(html[start]=='\"') start++; //it should be as we consider only text strings
				if(html[start]=='\0'){
					printf("unexpected error\n");
					exit(0);
				}
				//2.-searching for the end of the label
				end=start;
				while((html[end]!='\0')&&(html[end]!='\"')) end++;
				if(html[end]=='\"') end--; //it should be as we consider only text strings
				if(html[end]=='\0'){
					printf("unexpected error\n");
					exit(0);
				}
				//3.- copiamos la cadena
				mf_config[num-1]->field[total_fields]->label_f= return_sub_string(html, start, end);
				mf_config[num-1]->count_f+=1;// this is where total_fields is stored
			}else if(level==3){
				struct json_object **new_set_level_3;
				total_fields = mf_config[num-1]->count_f;
				total_objects =mf_config[num-1]->field[total_fields-1]->count_o;
				new_set_level_3= (struct json_object **) malloc((total_objects+1)* sizeof(struct json_object*));
				for(counter=0;counter< total_objects ;counter++)
					new_set_level_3[counter]=mf_config[num-1]->field[total_fields-1]->object[counter];
				free(mf_config[num-1]->field[total_fields-1]->object);
				mf_config[num-1]->field[total_fields-1]->object=new_set_level_3;
				mf_config[num-1]->field[total_fields-1]-> object[total_objects] =(struct json_object*) malloc(sizeof(struct json_object));
				//1.-searching for the begining of the label
				start=i+1;
				while((html[start]==' ')|| (html[start]=='\n')) start++;
				if(html[start]=='\"') start++; //it should be as we consider only text strings
				if(html[start]=='\0'){
					printf("unexpected error\n");
					exit(0);
				}
				//2.-searching for the end of the label
				end=start;
				while((html[end]!='\0')&&(html[end]!='\"') &&(html[start]!='\n')&&(html[start]!=',')) end++;
				if(html[end]=='\"')  end--; //it should be as we consider only text strings
				if(html[end]=='\0'){
					printf("unexpected error\n");
					exit(0);
				}
				//3.- copy the string LABEL
				mf_config[num-1]->field[total_fields-1]-> object[total_objects]-> label_o= return_sub_string(html, start, end);
				start=end;
				while((html[start]!=':')) start++;
				start++;
				while((html[start]==' ')|| (html[start]=='\n')) start++;
				if(html[start]=='\"') start++; //it should be as we consider only text strings
				if(html[start]=='\0'){
					printf("unexpected error\n");
					exit(0);
				}
				//2.-searching for the end of the string value
				end=start;
				while((html[end]!='\0')&&(html[end]!='\"') &&(html[end]!='\n')&&(html[end]!=',')) end++;
				if( (html[end]==',')) end--; //it should be as we consider only text strings
				if((html[end]=='\"') ) end--; //it should be as we consider only text strings
				if(html[end]=='\0'){
					printf("unexpected error\n");
					exit(0);
				}
				//3.- copy the string VALUE
				mf_config[num-1]->field[total_fields-1]-> object[total_objects]-> value_o= return_sub_string(html, start, end);
				mf_config[num-1]->field[total_fields-1]->count_o+=1;
			}
		}
	}
// 	printf("%s\n",NO_COLOUR);
	*in_mf_config= mf_config;
	return 0;
}//parse_mf_config_json


/**
 * print counters from JSON
 */
int print_counters_from_json(char *html){
	int level=0;
	for(int i=0;i<strlen(html);i++){
		if((html[i]=='[')||(html[i]=='{')){
// 			count_keys[level]=1;
			level++;
// 			printf("\n");
			for(int j=0;j<level;j++) printf("\t");
			int counter=count_mykeys_from_json(html, i);
			printf("%i-%i\n",level,counter);
		}else if((html[i]==']')||(html[i]=='}')){
// 			printf("\n");
// 			for(j=0;j<level;j++) printf("\t");
// 			printf("%i",count_keys[level-1]);
			level--;
// 		}else if((html[i]==',')){
// 			count_keys[level-1]+=1;
// 			printf("\n");
// 			for(j=0;j<level;j++) printf("\t");
		}
	}
	return 0;
}//print_counters_from_json

/**
 * print counters from JSON
 */
int print_counters_from_parsed_json(struct json_mf_config **mf_config,const unsigned int total_loaded_mf_configs){
	printf("%i-%i\n",1,total_loaded_mf_configs );
	for(int i=0;i<total_loaded_mf_configs;i++){
		printf("\t%i-%i\n",2,mf_config[i]->count_f);
		for(int total_fields=0; total_fields<mf_config[i]->count_f;total_fields++){
			printf("\t\t%i-%i\n",3,mf_config[i]->field[total_fields]-> count_o);
		}
	}
	return 0;
}//print_counters_from_json


int print_json(char *html){
	int i,j,level=0;
	for(i=0;i<strlen(html);i++){
		if((html[i]=='[')||(html[i]=='{')){
// 			count_keys[level]+=1;
			level++;
			printf("\n%s",NO_COLOUR);
			for(j=0;j<level;j++) printf("\t");
		}else if((html[i]==']')||(html[i]=='}')){
			printf("\n%s",NO_COLOUR);
			for(j=0;j<level;j++) printf("\t");
			level--;
		}else if((html[i]==',')){
// 			count_keys[level-1]+=1;
			printf("\n%s",NO_COLOUR);
			for(j=0;j<level;j++) printf("\t");
		}else if(i>0){
			if((html[i-1]=='\"')&&(html[i]==':'))
				printf("%s",NO_COLOUR);
			if((html[i-1]==':'))
				printf("%s",LIGHT_BLUE);
			if((html[i-1]=='[')||(html[i-1]=='{')||(html[i-1]==','))
				printf("%s",yellow);
		}
		printf("%c",html[i]);
	}
	return 0;
}


int print_parsed_json(struct json_mf_config **mf_config,const unsigned int total_loaded_mf_configs){
	int i,j,k;
	for(i=0;i<total_loaded_mf_configs;i++){
		if(i==0){
			printf("[\n");
		}else{
			printf(",");
		}
		for(j=0;j<mf_config[i]->count_f;j++){
			if(j==0){
				printf("\t{");
			}else{
				printf("\t,");
			}
			printf("\"%s%s%s\"\n",yellow,mf_config[i]->field[j]->label_f,NO_COLOUR);
			for(k=0;k<mf_config[i]->field[j]->count_o;k++){
				if(k==0){
					printf("\t\t{");
				}else{
					printf("\t\t,");
				}
				printf("\"%s%s%s\":",yellow,mf_config[i]->field[j]->object[k]->label_o,NO_COLOUR);
				printf("\"%s%s%s\"\n",LIGHT_BLUE,mf_config[i]->field[j]->object[k]->value_o,NO_COLOUR);
			}
			if(mf_config[i]->field[j]->count_o>0) printf("\t\t}\n");
		}
		printf("\t}\n");
	}
	printf("]%s\n\n\n",NO_COLOUR);
	return 0;
}

/**
 * @returns -1 if not found, in other case the num of struct in the mf_config array
 * */
int query_for_platform_parsed_json(const char *platform_id, struct json_mf_config **mf_config,const unsigned int total_loaded_mf_configs){
	for(int num =0; num <total_loaded_mf_configs;num++){
		for(int total_fields =0; total_fields<mf_config[num]->count_f; total_fields++){
			if(strcmp(mf_config[num]->field[total_fields]->label_f,"generic")==0){
				int total_objects= mf_config[num]->field[total_fields]->count_o;
				for(int counter=0;counter< total_objects ;counter++){
					if((strcmp(mf_config[num]->field[total_fields]-> object[counter]-> label_o,"platform_id")==0)&&
						(strcmp(mf_config[num]->field[total_fields]-> object[counter]-> value_o,platform_id)==0)){
							return num;
					}
				}
			}
		}
	}
	return -1;
}

/**
 * @returns NULL if not found, in other case the value for the label queried 
 * */
char *query_for_plugin_parsed_json(const unsigned int loaded_conf, char *plugin_id, struct json_mf_config **mf_config,const unsigned int total_loaded_mf_configs){
	char *result=NULL;
	if(loaded_conf >=total_loaded_mf_configs)
		return NULL;
	for(int total_fields =0; total_fields<mf_config[loaded_conf]->count_f; total_fields++){
		if(strcmp(mf_config[loaded_conf]->field[total_fields]->label_f,"plugins")==0){
			int total_objects= mf_config[loaded_conf]->field[total_fields]->count_o;
			for(int counter=0;counter< total_objects ;counter++){
				if(strcmp(mf_config[loaded_conf]->field[total_fields]-> object[counter]-> label_o,plugin_id)==0){
					concat_and_free(&result, mf_config[loaded_conf]->field[total_fields]-> object[counter]-> value_o);
// 					log_error("result is %s\n",mf_config[loaded_conf]->field[total_fields]-> object[counter]-> value_o);
// 					fflush(stdout);
					return result;
				}
			}
		}
	}
	return NULL;
}
