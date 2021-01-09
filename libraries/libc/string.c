#include <stddef.h>
#include <stdint.h>
#include <string.h>

int memcmp(void * s1, void * s2, size_t n){
  uint8_t * p1 = (uint8_t *) s1, * p2 = (uint8_t *) s2;
  while(n--){
    if( *p1 != *p2 ){
      return *p1 - *p2;
    } else {
      p1++,p2++;
    }
  }
  return 0;
}

void * memmove(void * dst, void * src, size_t len){
  size_t i;
  if((uint32_t) dst < (uint32_t) src){
    return memcpy(dst, src, len);
  }

  if ((uint32_t)dst % sizeof(long) == 0 &&
     (uint32_t)src % sizeof(long) == 0 &&
     len % sizeof(long) == 0){
       long *d = dst;
       long *s = src;
       for(i = len / sizeof(long); i > 0; i--){
         d[i-1] = s[i-1];
       }
  } else {
    char *d = dst;
    const char *s = src;
    for(i = len; i > 0; i--){
      d[i-1] = s[i-1];
    }
  }

  return dst;
}
// copies num bytes from source to destination
void * memcpy(void * destination, void * source, size_t num){
  uint8_t * dst = (uint8_t *) destination;
  uint8_t * src = (uint8_t *) source;
  size_t i = 0;
  while(i < num){
    dst[i] = src[i];
    i++;
  }
  return (void *) dst;
}

// sets num bytes of ptr to value
void * memset(void * ptr, int value, size_t num){
  uint8_t * p = (uint8_t *) ptr;
  size_t i = 0;
  while(i < num){
    p[i] = value;
    i++;
  }
  return (void *) p;
}

char * strchr(char * string, char character){
  char c = character;
	for (;; ++string) {
		if (*string == c)
			return((char *)string);
		if (!*string)
			return ((char *)NULL);
	}
}

// counts characters in a string
size_t strlen(char * str){
  size_t i = 0;
  while(str[i] != 0){
    i++;
  }
  return (size_t) i;
}

// copies source into destination for len characters up to null
char * strcpy(char * destination, char * source){
  size_t i = 0;
  while((destination[i] = source[i]) != '\0'){
    i++;
  }
  return (char *) destination;
}

// copies n characters from source to destination
char * strncpy(char * destination, char * source, size_t n){
  size_t i = 0;
  while(i < n && source[i] != '\0'){
    destination[i] = source[i];
    i++;
  }
  return (char *) destination;
}

// compares 2 strings
int strcmp(char * str1, char * str2){
  while (*str1 == *str2++){
    if(*str1++ == 0){
      return 0;
    }
  }
	return (*(const unsigned char *)str1 - *(const unsigned char *)--str2);
}

// compares length caracters of str1 and str2
// returns 0 if equal and -1 or 1 if not
int strncmp(char * str1, char * str2, size_t n){
  int i = 0;
  while( (*str1 != '\0') && (*str2 != '\0') && i < n){
    if(*str1 > *str2){
      return 1;
    } else if(*str2 > *str1){
      return -1;
    }
    str1++;
    str2++;
    i++;
  }
  return 0;
}

// concatenates src onto destination
char * strcat(char * dest, char * src){
  char * dest_save = dest;
  char * str = (char *) src;
  while(*dest++);
  dest--;
  while((*dest++ = *str++) != 0);
  return dest_save;
}

// appends c char to the end of str
char * sappend(char * str, char c){
  int len = strlen(str);
  str[len] = c;
  str[len + 1] = '\0';
  return str;
}

// reverses string
void strrev(char * s, char * l){
  int i, j;
	char c;
	for (i = 0, j = l-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

// puts hex value of uinteg to string
int sputhex(char * str, uint32_t uinteg){
  uint8_t no_ze = 1;
  signed int temp;
  int str_i = 1;

  str[0] = '0';
  str[1] = 'x';

  for(int i = 28; i > 0; i -= 4){
    temp = (uinteg >> i) & 0xF;
    if ( (temp == 0) && (no_ze != 0) ){
      continue;
    }
    if (temp >= 0xA){
      no_ze = 0;
      str[++str_i] = (char )(temp - 0xA + 'A');
    } else {
      no_ze = 0;
      str[++str_i] = (char) (temp + '0');
    }
  }

  temp = uinteg & 0xF;

  if (temp >= 0xA){
    str[++str_i] = (char) (temp - 0xA + 'A');
  } else {
    str[++str_i] = (char) (temp + '0');
  }
  str_i += 1;
  return str_i;
}

// inserts unsigned integer to the string
int sputuint(char * str, uint32_t uinteg){
  uint32_t num;
  uint32_t divisor = 1000000000;
  uint32_t index = 0;
  while( (uinteg / divisor == 0) && (divisor >= 10) ){
    divisor /= 10;
  }
  num = uinteg;
  while( divisor >= 10){
    str[index++] = ( (uint8_t) ( (int) '0' + num / divisor) );
    num = num % divisor;
    divisor /= 10;
  }
  str[index++] = (uint8_t) ( (int) '0' + num);
  str[index] = 0;
  num = 0;
  return index;
}

// puts signed decimal integer to the screen
int sputint(char * str, int num){
  int written = 0;
  if(num >= 0){
    written = sputuint(str, (uint32_t) num);
  } else {
    str[0] = '-';
    written = sputuint(&str[1], (uint32_t) -num);
    written++;
  }
  return written;
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

int sputfpoint(char * str, double num){
  int exponent = 0;
  int places = 0;
  int width = 4;

  if (num == 0.0) {
      str[0] = '0';
      str[1] = '\0';
      return 3;
  }

  if (num < 0.0) {
    *str++ = '-';
    num = -num;
  }

  while(num >= 1.0){
    num /= 10.0;
    exponent++;
  }

  while(num < 0.1){
    num *= 10.0;
    --exponent;
  }

  while (exponent > 0) {
    int digit = num * 10;
    *str++ = digit + '0';
    num = num * 10 - digit;
    ++places;
    --exponent;
  }

  if (places == 0)
      *str++ = '0';

  *str++ = '.';

  while (exponent < 0 && places < width) {
    *str++ = '0';
    --exponent;
    ++places;
  }

  while (places < width) {
    int digit = num * 10.0;
    *str++ = digit + '0';
    num = num * 10.0 - digit;
    ++places;
  }
  *str = '\0';

  return (strlen(str) + 1);
}

unsigned long strtoul(char * cp, char **endp, uint32_t base){
  unsigned long result = 0;
  unsigned long value;

  if(!base){
      base = 10;
      if(*cp == '0'){
          base = 8;
          cp++;
          if ((*cp == 'x') && isxdigit(cp[1])) {
              cp++;
              base = 16;
          }
      }
  }
  while(isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base){
      result = result * base + value;
      cp++;
  }
  if(endp)
      *endp = (char *)cp;
  return result;
}

long strtol(char * cp, char **endp, uint32_t base){
  if(*cp=='-')
      return -strtoul(cp+1,endp,base);
  return strtoul(cp,endp,base);
}

unsigned long long strtoull(char * cp, char **endp, unsigned int base){
  unsigned long long result = 0;
  unsigned long long value;

  if(!base){
      base = 10;
      if (*cp == '0') {
          base = 8;
          cp++;
          if ((*cp == 'x') && isxdigit(cp[1])) {
              cp++;
              base = 16;
          }
      }
  }
  while(isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp) ? toupper(*cp) : *cp)-'A'+10) < base){
      result = result*base + value;
      cp++;
  }
  if (endp)
      *endp = (char *)cp;
  return result;
}

long long strtoll(char * cp, char **endp, unsigned int base){
  if(*cp=='-')
      return -strtoull(cp+1,endp,base);
  return strtoull(cp,endp,base);
}
