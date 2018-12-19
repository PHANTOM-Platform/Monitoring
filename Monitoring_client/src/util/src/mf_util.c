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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include <malloc.h>
#include <math.h>

#define SUCCESS 0
#define FAILED 1


/*******************************************************************************
* String functions
******************************************************************************/
char myBuffer[255];
char myfBuffer[255];

/** Function for increase dynamically a string concatenating strings at the end
* It free the memory of the first pointer if not null
* @return s1 <-- s1 + s2 */
char* concat_and_free(char **s1, const char *s2){
	char *result = NULL;
	unsigned int new_lenght= strlen(s2)+1; //+1 for the null-terminator;
	if(*s1 != NULL){
		new_lenght+= strlen(*s1);//current lenght
		if(new_lenght> malloc_usable_size(*s1)){
			result = (char *) malloc(new_lenght);
			if(result==NULL) {
				printf("Failed to allocate memory.\n");
				exit(1);
			}
			strcpy(result, *s1);
			free(*s1);
		}else
			result = *s1;
	}else{
		result = (char *) malloc(new_lenght);
		if(result==NULL) {
			printf("Failed to allocate memory.\n");
			exit(1);
		}
		result[0]='\0';
	}
	*s1 = result;
	strcat(result, s2);
	return result;
}

char* itoa(int i, char b[]){
	char const digit[] = "0123456789";
	char* p = b;
	if(i<0){
		*p++ = '-';
		i *= -1;
	}
	int shifter = i;
	do{ //Move to where representation ends
		++p;
		shifter = shifter/10;
	}while(shifter);
	*p = '\0';
	do{ //Move back, inserting digits as u go
		*--p = digit[i%10];
		i = i/10;
	}while(i);
	return b;
}


char* llitoa(const long long int i, char b[]){
	char const digit[] = "0123456789";
	char* p = b;
	long long int input = i;
	if(input<0){
		*p++ = '-';
		input *= -1;
	}
	int shifter = input;
	do{ //Move to where representation ends
		++p;
		shifter = shifter/10;
	}while(shifter);
	*p = '\0';
	do{ //Move back, inserting digits as u go
		*--p = digit[input%10];
		input = input/10;
	}while(input);
	return b;
}


#if defined CHAR_BIT
	// All defined OK so do nothing
#else
	#define CHAR_BIT 8
#endif

//A base-10 representation of a n-bit binary number takes up to n*log10(2) + 1 digits.
//10/33 is slightly more than log10(2). +1
#define INT_DECIMAL_STRING_SIZE(int_type) ((CHAR_BIT*sizeof(int_type)-1)*10/33+3)

char *llint_to_string_alloc(long long int x, char b[]) {
	long long int i = x;
	unsigned int buf_size = INT_DECIMAL_STRING_SIZE(long long int);// sizeof buf
	char buf[INT_DECIMAL_STRING_SIZE(long long int)];
	char *p = &buf[buf_size - 1]; // point to the end
	*p = '\0';
	if (i >= 0)
		i = -i;
	do {
		p--;
		*p = (char) ('0' - i % 10);
		i /= 10;
	} while (i);
	if (x < 0) {
		p--;
		*p = '-';
	}
	size_t len = (size_t) (&buf[buf_size] - p);
	memcpy(b, p, len);
	return b;
}

/**
* Function for increase dynamically a string concatenating strings at the end
* It free the memory of the first pointer if not null
* @return all the strings concatenated
**/
char* myconcat(char **s1,const char *a1,const char *a2,const char *a3,const char *a4,const char *a5){
	char *result = NULL;
	unsigned int new_lenght= strlen(a1)+strlen(a2)+strlen(a3)+strlen(a4)+strlen(a5)+1; //+1 for the null-terminator;
	if(*s1 != NULL){
		new_lenght+= strlen(*s1);//current lenght
		if(new_lenght> malloc_usable_size(*s1)){
			result = (char *) malloc(new_lenght);
			if(result==NULL) {
				fprintf(stderr, "Failed to allocate memory.\n");
				return 0;
			}
			strcpy(result, *s1);
			free(*s1);
		}else{
			result = *s1;
		}
	}else{
		result = (char *) malloc(new_lenght);
		if(result==NULL) {
			fprintf(stderr, "Failed to allocate memory.\n");
			return 0;
		}
		result[0]='\0';
	}
	*s1 = result;
	strcat(result, a1);
	strcat(result, a2);
	strcat(result, a3);
	strcat(result, a4);
	strcat(result, a5);
	return result;
}

/** @returns the reversed of the string 'str' of length 'len' **/
void reverse(char *str, int len) {
	int i=0, j=len-1, temp;
	while (i<j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++; j--;
	}
}

/**
* Converts a given integer x to string str[].  d is the number
* of digits required in output. If d is more than the number
* of digits in x, then 0s are added at the beginning.
 @returns the length of the string
**/
int intToStr(int x, char str[], int d) {
	int i = 0;
	while (x) {
		str[i++] = (x%10) + '0';
		x = x/10;
	}
	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
		str[i++] = '0';
	reverse(str, i);
	str[i] = '\0';
	return i;
}

/** ftoa(n, res, afterpoint)
* n          --> Input Number
* res[]      --> Array where output string to be stored
* afterpoint --> Number of digits to be considered after point.
*
* For example ftoa(1.555, str, 2) should store "1.55" in res and
* ftoa(1.555, str, 0) should store "1" in res.
*/

/** @return the conversion of a floating point number into string. */
char *ftoa(const float n, const int afterpoint) {
	// Extract integer part
	int ipart = (int)n;
	
	// Extract floating part
	float fpart = n - (float)ipart;
	// convert integer part to string
// 	strcpy(myfBuffer,mintToStr(ipart, 0));
// 	int i=strlen(myfBuffer);
	int i = intToStr(ipart, myfBuffer, 0);
	// check for display option after point
	if (afterpoint != 0) {
		myfBuffer[i] = '.';  // add dot
		myfBuffer[i+1] = '\0';  // add dot
		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter is needed
		// to handle cases like 233.007
		fpart = fpart * pow(10, afterpoint);
// 		strcat(myfBuffer,mintToStr((int)fpart, afterpoint));
		intToStr((int)fpart, myfBuffer + i + 1, afterpoint);
	}
	return myfBuffer;
}

/** Converts a given integer x to string str[].  d is the number
* of digits required in output. If d is more than the number
* of digits in x, then 0s are added at the beginning.
 @return the string */
const char* mintToStr(const int x, const int d) {
	int i = 0;
	int temp=x;
	if(temp==0) myBuffer[i++] = '0';
	while (temp) {
		myBuffer[i++] = (temp%10) + '0';
		temp = temp/10;
	}
	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
		myBuffer[i++] = '0';
	reverse(myBuffer, i);
	myBuffer[i] = '\0';
	return myBuffer;
}

const char* llintToStr(const long long int x, const int d) {
	int i = 0;
	long long int temp=x;
	if(temp==0) myBuffer[i++] = '0';
	while (temp) {
		myBuffer[i++] = (temp%10) + '0';
		temp = temp/10;
	}
	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
		myBuffer[i++] = '0';
	reverse(myBuffer, i);
	myBuffer[i] = '\0';
	return myBuffer;
}

//Function for increase dynamically a string concatenating strings at the end
//It free the memory of the first pointer if not null
// returns: s1 <- s1 + s2 + s3 + s4
char* concat_strings(char **s1, const char *s2, const char *s3, const char *s4){
	char *result = NULL;
	unsigned int new_lenght= strlen(s2)+strlen(s3)+strlen(s4)+1; //+1 for the null-terminator;
	if(*s1 != NULL){
		new_lenght+= strlen(*s1);//current lenght
		if(new_lenght> malloc_usable_size(*s1)){
			result = (char *) malloc(new_lenght);
			if(result==NULL) {
				printf("Failed to allocate memory.\n");
				exit(1);
			}
			strcpy(result, *s1);
			free(*s1);
		}else{
			result = *s1;
		}
	}else{
		result = (char *) malloc(new_lenght);
		if(result==NULL) {
			printf("Failed to allocate memory.\n");
			exit(1);
		}
		result[0]='\0';
	}
	*s1 = result;
	strcat(result, s2);
	strcat(result, s3);
	strcat(result, s4);
	return result;
}


/*******************************************************************************
* Arithmetic functions
******************************************************************************/
int add(int a, int b, _Bool *overflowFlag) {
        int c = a + b;
        *overflowFlag = ((a ^ b) >= 0) & ((a ^ c) < 0);
        return c;
}

long int ladd( long int a, long int b, _Bool *overflowFlag) {
        long int c = a + b;
        *overflowFlag = ((a ^ b) >= 0) & ((a ^ c) < 0);
        return c;
}

long long int lladd( long long int a, long long int b, _Bool *overflowFlag) {
        long long int c = a + b;
        *overflowFlag = ((a ^ b) >= 0) & ((a ^ c) < 0);
        return c;
}
