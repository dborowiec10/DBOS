#ifndef _STDLIB_H
#define _STDLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef void (* at_exit_callb_t)(void);

#define NULL (void *) 0

#define EXIT_FAILURE 1

#define EXIT_SUCCESS 0

typedef struct at_exit_func {
  at_exit_callb_t callback;
  struct at_exit_func * next;
} at_exit_t;

static at_exit_t * caller = NULL;

void abort(void) __attribute__ ((noreturn));

int atexit(void (*)(void));

int atoi(const char*);

double atof(const char*);

int itoa(int value, char * s);

double strtod(char *, char **ptr);

void free(void*);

char* getenv(const char*);

void* malloc(size_t);

void* realloc(void *, size_t);

#ifdef __cplusplus
}
#endif

#endif
