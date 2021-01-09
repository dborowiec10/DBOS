#include <stdint.h>
#include <stddef.h>
#include <kernel/util/string.h>
#include <kernel/util/kstdio.h>
#include <kernel/memory/memory_heap.h>

// calculates size of a string
size_t strlen(char * string){
  int i = 0;
  while(string[i] != 0){
    i++;
  }
  return i;
}

// compares str1 and str2, returns 0 if equal or 1 if not
int strcmp(char * str1, char * str2){
  while( (*str1 != '\0') || (*str2 != '\0') ){
    if( (*str1 > *str2) || (*str2 > *str1)){
      return 1;
    }
    str1++;
    str2++;
  }
  return 0;
}

// compares length caracters of str1 and str2
// returns 0 if equal and -1 or 1 if not
int strncmp(char * str1, char * str2, int length){
  unsigned char * string1 = (unsigned char *) str1;
  unsigned char * string2 = (unsigned char *) str2;
  for(int i = 0; i < length; i++){
    if(string1[i] < string2[i]){
      return -1;
    }
    if(string2[i] < string1[i]){
      return 1;
    }
    if(string1[i] == 0){
      return 0;
    }
  }
  return 0;
}

// copies source to destination
void strcpy(char * destination, char * source){
  int i = 0;
  while((destination[i] = source[i]) != '\0'){
    i++;
  }
}

// copies len characters from source to destination
void strncpy(char * destination, char * source, int len){
  int i = 0;
  while((destination[i] = source[i]) != '\0' && i < len){
    i++;
  }
}

// duplicates string, returns duplicate
char * strdup(char * string){
  int string_length = strlen(string) + 1;
  char * tmp = (char *) kern_malloc(string_length);
  memcpy_8((uint8_t *) tmp, (uint8_t *) string, string_length);
  tmp[string_length] = 0;
  return tmp;
}

// concatenates string at the end of dest
char * strcat(char * dest, char * string){
  while(*dest++);
  dest--;
  while((*dest++ = *string++) != 0);
  return dest;
}

// finds first occurence of a character in a string
// returns character + remaining string
char * strchr(char * string, char character){
  int i = 0;
  while (*string != (char) character){
    if(!(*string++)){
      return NULL;
    }
  }
  return (char *) string;
}

// finds last occurence of the character within the string
// and returns character + remaining string
char * strrchr(char * string, char character){
  char * ret = NULL;
  do {
    if(*string == character){
      ret = string;
    }
  } while(*string++);
  return ret;
}

// checks string1 for first occurence of any char in string2
// returns number of chars in string1 which were read before this first occurence
int strcspn(char * string1, char * string2){
  int len = 0;
  while(*string1){
    char * temp = strchr(string2, *string1);
    if(temp){
      return len;
    } else {
      string1++;
      len++;
    }
  }
  return len;
}

// calculates length of the portion of string1
// which consists of characters that are in string2.
int strspn(char * string1, char * string2){
  int len = 0;
  while(*string1 && strchr(string2, *string1++)){
    len++;
  }
  return len;
}

// tokenizes a string based on delimiters
char * strtok(char * string, const char * delimiters){
  static char * stored_string;
  char character;
  if (string == NULL){
    string = stored_string;
  }
  do {
     if ((character = *string++) == '\0'){
       return NULL;
     }
  } while (strchr((char *) delimiters, character));
  --string;
  stored_string = string + strcspn(string, (char *) delimiters);
  if (*stored_string != 0){
    *stored_string++ = 0;
  }
  return string;
}

// turns ascii based number into an int from a str
int atoi(char * str) {
    int acum = 0;
    int factor = 1;

    if(*str == '-') {
        factor = -1;
        str++;
    }

    while((*str >= '0')&&(*str <= '9')) {
      acum = acum * 10;
      acum = acum + (*str - 48);
      str++;
    }
    return (factor * acum);
}

// finds first occurence of needle in the haystack and returns a pointer to it
char * strstr(char * haystack, char * needle){
  if(haystack == NULL || needle == NULL || haystack[0] == '\0' || needle[0] == '\0'){
    return NULL;
  }
  char * oper1 = haystack;
  char * oper2 = needle;
  char * adv = haystack;
  // if needle is in haystack, it will be in the worst case at haystack - needle position
  while(*++oper2){
    adv++;
  }
  // while we still have some chars
  while(*adv){
    // save the haystack
    char * oper1s = oper1;
    // ensure we are working on a fresh needle
    oper2 = needle;
    // keep looping until either operand is null or they are not equal
    while(*oper1 != '\0' && *oper2 != '\0' && *oper1 == *oper2){
      oper1++;
      oper2++;
    }
    // if needle is now null, we looped right to the end of it
    // we found it
    if(*oper2 == '\0'){
      return oper1s;
    }
    // advance haystack and the diff advancer
    oper1 = oper1s + 1;
    adv++;
  }
  // worst case, nothing found, return null
  return NULL;
}
