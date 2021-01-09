#ifndef STRING_H
#define STRING_H
#include <stdint.h>
#include <stddef.h>

size_t strlen(char * string);

int strcmp(char * str1, char * str2);

int strncmp(char * str1, char * str2, int length);

void strcpy(char * destination, char * source);

void strncpy(char * destination, char * source, int len);

char * strdup(char * string);

char * strcat(char * dest, char * string);

char * strchr(char * string, char character);

char * strrchr(char * string, char character);

int strcspn(char * string1, char * string2);

int strspn(char * string1, char * string2);

// char * strtok(char * string, char * delimiter);
char * strtok(char * string, const char * delimiters);

int atoi(char * c);

char * strstr(char * haystack, char * needle);

#endif
