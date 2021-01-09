#ifndef _STRING_H
#define _STRING_H
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void * memcpy(void * destination, void * source, size_t num);

void * memset(void * ptr, int value, size_t num);

void * memmove(void * dst, void * src, size_t len);

int memcmp(void * s1, void * s2, size_t n);

size_t strlen(char * str);

char * strcpy(char * destination, char * source);

char * strncpy(char * destination, char * source, size_t n);

char * strchr(char * string, char character);

int strcmp(char * str1, char * str2);

int strncmp(char * str1, char * str2, size_t n);

char * strcat(char * dest, char * src);

char * sappend(char * str, char c);

void strrev(char * s, char * l);

int sputhex(char * str, uint32_t uinteg);

int sputuint(char * str, uint32_t uinteg);

int sputint(char * str, int num);

int sputfpoint(char * str, double num);

int strcspn(char * string1, char * string2);

unsigned long strtoul(char * cp, char **endp, uint32_t base);

long strtol(char * cp, char **endp, uint32_t base);

unsigned long long strtoull(char * cp, char **endp, unsigned int base);

long long strtoll(char * cp, char **endp, unsigned int base);

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define isxdigit(c) (isdigit(c) || \
    (((c) >= 'a') && ((c) <= 'f')) || \
    (((c) >= 'A') && ((c) <= 'F')))

#define islower(c) (((c) >= 'a') && ((c) <= 'z'))

#define toupper(c) (islower(c) ? ((c) - ('a' - 'A')) : (c))

#define isspace(c) ( (c) == ' ' || (c) == '\n' || (c) == '\t')

#ifdef __cplusplus
}
#endif

#endif
