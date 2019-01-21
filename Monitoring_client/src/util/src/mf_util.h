/*
* Copyright 2018 High Performance Computing Center, Stuttgart
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
*/

#ifndef COLORS_
#define COLORS_
	#define BLUE "\033[0;34m"
//	#define LIGHT_GRAY "\033[0;37m"
	#define LIGHT_GREEN "\033[1;32m"
	#define LIGHT_BLUE "\033[1;34m"
	#define LIGHT_CYAN "\033[1;36m"
	#define yellow "\033[1;33m"
	#define WHITE "\033[1;37m"
	#define RED "\033[0;31m"
//	#define marron "\033[2;33m"
	#define NO_COLOUR "\033[0m"
//	#define white "\033[0;0m"
#endif

/*******************************************************************************
* String functions
******************************************************************************/
/** Function for increase dynamically a string concatenating strings at the end
* It free the memory of the first pointer if not null
* @return s1 <-- s1 + s2 */
char* concat_and_free(char **s1, const char *s2);

char* itoa(int i, char b[]);

char* llitoa(const long long int i, char b[]);

char *llint_to_string_alloc(long long int x, char b[]);

/**
* Function for increase dynamically a string concatenating strings at the end
* It free the memory of the first pointer if not null
* @return all the strings concatenated
**/
char* myconcat(char **s1,const char *a1,const char *a2,const char *a3,const char *a4, const char *a5);

/**
* Converts a given integer x to string str[].  d is the number
* of digits required in output. If d is more than the number
* of digits in x, then 0s are added at the beginning.
* returns: the length of the string
**/
int intToStr(int x, char str[], int d);

/** @return the conversion of a floating point number into string. */
char *ftoa(const float n, const int afterpoint);

const char* mintToStr(const int x, const int d);

const char* llintToStr(const long long int x, const int d);

//Function for increase dynamically a string concatenating strings at the end
//It free the memory of the first pointer if not null
// returns: s1 <- s1 + s2 + s3 + s4
char* concat_strings(char **s1, const char *s2, const char *s3, const char *s4);

/*******************************************************************************
* Arithmetic functions
******************************************************************************/
int add(int a, int b, _Bool *overflowFlag) ;

long int ladd( long int a, long int b, _Bool *overflowFlag) ;

long long int lladd( long long int a, long long int b, _Bool *overflowFlag);
