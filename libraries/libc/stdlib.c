#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <alloc.h>
#include <scalls.h>
#include <string.h>

// aborts the program
void __attribute__ ((noreturn)) abort(void) {
  exit(-10);
}

// register callback to be called at the end of the program
int atexit(at_exit_callb_t callback){
  at_exit_t * call = (at_exit_t *) malloc(sizeof(at_exit_t));

  if(call == NULL){
    return -1;
  }

  call->next = caller; // caller defined in stdlib.h
  call->callback = callback;
  caller = call;

  return 0;
}

// returns integer representation of the number specified in the string
int atoi(const char * str){
  int acum = 0;
  int factor = 1;
  size_t len = strlen((char *)str);
  size_t cur_len = 0;

  if(*str == '-') {
      factor = -1;
      str++;
  }

  while((*str >= '0') && (*str <= '9')) {
    acum = acum * 10;
    acum = acum + (*str - 48);
    str++;
    cur_len++;
  }

  if(cur_len != len){
    return -1;
  } else {
    return (factor * acum);
  }

}

// returns double representation of the number specified in the string
double atof(const char * s){
  double a = 0.0;
  int e = 0;
  int c;
  while((c = *s++) != '\0' && isdigit(c)){
    a = a * 10.0 + (c - '0');
  }

  if(c == '.'){
    while((c = *s++) != '\0' && isdigit(c)){
      a = a * 10.0 + (c - '0');
      e = e - 1;
    }
  }

  if(c == 'e' || c == 'E'){
    int sign = 1;
    int i = 0;
    c = *s++;

    if(c == '+'){
      c = *s++;
    } else if(c == '-'){
      c = *s++;
      sign = -1;
    }

    while(isdigit(c)){
      i = i * 10 + (c - '0');
      c = *s++;
    }

    e += i * sign;
  }

  while(e > 0){
    a *= 10.0;
    e--;
  }

  while(e < 0){
    a *= 0.1;
    e++;
  }

  return a;
}

int itoa(int value, char * s){
  char * begin = s;
	char * rev = s;
	int n = value;

	if(value < 0 ){
		n = -value;
		*s++ = '-';
		rev++;
	}

	do {
		*s++ = (n % 10) + '0';
	} while ((n /= 10) > 0);

	strrev( rev, s - rev);
  s[strlen(s)] = '\0';
  return s - begin;
}

double strtod (char * str, char ** ptr){
  char *p;
  if(ptr == (char **) 0){
    return atof(str);
  }

  p = str;

  while(isspace(*p)){
    ++p;
  }

  if(*p == '+' || *p == '-'){
    ++p;
  }

  if((p[0] == 'i' || p[0] == 'I') && (p[1] == 'n' || p[1] == 'N') && (p[2] == 'f' || p[2] == 'F')){
    if((p[3] == 'i' || p[3] == 'I') && (p[4] == 'n' || p[4] == 'N') && (p[5] == 'i' || p[5] == 'I') && (p[6] == 't' || p[6] == 'T') && (p[7] == 'y' || p[7] == 'Y')){
      *ptr = p + 8;
	    return atof (str);
    } else {
	    *ptr = p + 3;
	    return atof (str);
    }
  }

  if((p[0] == 'n' || p[0] == 'N') && (p[1] == 'a' || p[1] == 'A') && (p[2] == 'n' || p[2] == 'N')){
    p += 3;
    if(*p == '('){
	    ++p;

  	  while(*p != '\0' && *p != ')'){
        ++p;
      }

      if(*p == ')'){
        ++p;
      }
	  }
    *ptr = p;
    return atof (str);
  }

  if(isdigit(*p) || *p == '.'){
    int got_dot = 0;
    while (isdigit(*p) || (!got_dot && *p == '.')){
      if(*p == '.'){
        got_dot = 1;
      }
    	++p;
    }

    if(*p == 'e' || *p == 'E'){
      int i;
	    i = 1;
	    if(p[i] == '+' || p[i] == '-'){
        ++i;
      }
	    if(isdigit (p[i])){
	      while (isdigit (p[i])){
          ++i;
        }
	      *ptr = p + i;
	      return atof (str);
      }
    }
    *ptr = p;
    return atof (str);
  }
  /* Didn't find any digits.  Doesn't look like a number.  */
  *ptr = str;
  return 0.0;
}

// unimplemented
char * getenv(const char * name){
  return (char *) NULL;
}

// frees allocated memory
void free(void * ptr){
  alloc_free((uint32_t *) ptr);
}

// allocates size bytes of memory
void * malloc(size_t size){
  return (void *) alloc_allocate((uint32_t) size);
}

// reallocates size bytes at given memory area
void * realloc(void * ptr, size_t size){
  if(size == 0){
    free(ptr);
    return NULL;
  }
  if(ptr == NULL){
    return malloc(size);
  }
  return (void *) alloc_reallocate((uint32_t *) ptr, (uint32_t) size);
}
