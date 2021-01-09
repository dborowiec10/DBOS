#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <scalls.h>
#include <sys/ioctl.h>
#include <unistd.h>

// file representing standard in stream
FILE stdin_s = {
  .id = 0,
  .seek_index = 0,
  .mode = MODE_R
};

// file representing standard out stream
FILE stdout_s = {
  .id = 0,
  .seek_index = 0,
  .mode = MODE_R
};

// file representing standard error stream
FILE stderr_s = {
  .id = 0,
  .seek_index = 0,
  .mode = MODE_R
};

FILE * stdin = NULL;

FILE * stderr = NULL;

FILE * stdout = NULL;

// initialise stdio
void stdio_init(){
  stdin = &stdin_s;
  stdout = &stdout_s;
  stderr = &stderr_s;
}

// prints formatted list of arguments to stdout
int printf(char * format, ...){
  va_list arg;

  int ret;

  va_start(arg, format);

  ret = vfprintf(stdout, format, arg);

  va_end(arg);

  return ret;
}

// writes formatted list of arguments to file
int fprintf(FILE * file, char * format, ...){
  va_list arg;

  int ret;

  va_start(arg, format);

  ret = vfprintf(file, format, arg);

  va_end(arg);

  return ret;
}

// writes formatted list of arguments to string
int sprintf(char * str, char * format, ...){
  va_list arg;

  int ret;

  va_start(arg, format);

  ret = vsprintf(str, format, arg);

  va_end(arg);

  return ret;
}

int snprintf(char * str, size_t size, char * format, ...){
  va_list args;
  int i;
  va_start(args, format);
  i = vsnprintf(str, size, format, args);
  va_end(args);
  return i;
}

int vsnprintf(char * str, size_t size, char * format, va_list arg){
  int i = 0;
  int j = 0;
  char * str_temp = NULL;
  while(format[i] & j < size){
    if(format[i] == '%'){
      i++;
      if(format[i] == 's'){
        str_temp = va_arg(arg, char *);
        strcpy(&str[j], str_temp);
        j += strlen(str_temp);

      } else if(format[i] == 'c'){
        str[j++] = (char) va_arg(arg, int);

      } else if(format[i] == 'd'){
        j += sputint(&str[j], va_arg(arg, int));

      } else if(format[i] == 'u'){
        j += sputuint(&str[j], va_arg(arg, uint32_t));

      } else if(format[i] == 'x'){
        j += sputhex(&str[j], va_arg(arg, uint32_t));

      } else {
        str[j++] = format[i];

      }

    } else {
      str[j++] = format[i];
    }
    i++;
  }
  return j;
}


// writes formatted va_list of arguments to string
int vsprintf(char * str, char * format, va_list arg){
  int i = 0;
  int j = 0;
  char * str_temp = NULL;
  while(format[i]){
    if(format[i] == '%'){
      i++;
      if(format[i] == 's'){
        str_temp = va_arg(arg, char *);
        strcpy(&str[j], str_temp);
        j += strlen(str_temp);

      } else if(format[i] == 'c'){
        str[j++] = (char) va_arg(arg, int);

      } else if(format[i] == 'd'){
        j += sputint(&str[j], va_arg(arg, int));

      } else if(format[i] == 'u'){
        j += sputuint(&str[j], va_arg(arg, uint32_t));

      } else if(format[i] == 'x'){
        j += sputhex(&str[j], va_arg(arg, uint32_t));

      } else if(format[i] == 'f'){
        j += sputfpoint(&str[j], va_arg(arg, double));

      } else {
        str[j++] = format[i];

      }

    } else {
      str[j++] = format[i];
    }
    i++;
  }
  return j;
}

// writes formatted va_list of arguments to file
int vfprintf(FILE * file, char * format, va_list arg){
  int i = 0;
  int j = 0;
  char * str_temp = NULL;
  char num_temp_str[100] = {0};

  while(format[i]){
    if(format[i] == '%'){
      i++;

      if(format[i] == 's'){
        str_temp = va_arg(arg, char *);
        fputs(str_temp, file);
        j += strlen(str_temp);

      } else if(format[i] == 'c'){
        fputc((char) va_arg(arg, int), file);
        j++;

      } else if(format[i] == 'd'){
        memset(num_temp_str, 0x00, 100);
        j += sputint(num_temp_str, va_arg(arg, int));
        fputs(num_temp_str, file);

      } else if(format[i] == 'u'){
        memset(num_temp_str, 0x00, 100);
        j += sputuint(num_temp_str, va_arg(arg, uint32_t));
        fputs(num_temp_str, file);

      } else if(format[i] == 'x'){
        memset(num_temp_str, 0x00, 100);
        j += sputhex(num_temp_str, va_arg(arg, uint32_t));
        fputs(num_temp_str, file);

      } else if(format[i] == 'f'){
        memset(num_temp_str, 0x00, 100);
        j += sputfpoint(num_temp_str, va_arg(arg, double));
        fputs(num_temp_str, file);

      } else {
        fputc(format[i], file);
        j++;

      }

    } else {
      fputc(format[i], file);
      j++;
    }
    i++;
  }
  return j;
}

int skip_atoi(char **s){
  int i = 0;
  while (isdigit(**s)){
    i = i * 10 + *((*s)++) - '0';
  }
  return i;
}

int sscanf(char * buf, char * fmt, ...){
	va_list args;
	int i;
	va_start(args,fmt);
	i = vsscanf(buf,fmt,args);
	va_end(args);
	return i;
}

int vsscanf(char * buf, char * format, va_list args){
  char * str = buf;
  char * next;
  char digit;
  int num = 0;
  int qualifier;
  int base;
  int field_width;
  int is_sign = 0;

  while(*format && *str){

    if(isspace(*format)){
      while(isspace(*format)){
        ++format;
      }
      while(isspace(*str)){
        ++str;
      }
    }

    if(*format != '%' && *format) {
      if(*format++ != *str++){
        break;
      }
      continue;
    }

    if(!*format){
      break;
    }
    ++format;

    if(*format == '*'){
      while(!isspace(*format) && *format){
        format++;
      }
      while(!isspace(*str) && *str){
        str++;
      }
      continue;
    }

    field_width = -1;
    if(isdigit(*format)){
      field_width = skip_atoi(&format);
    }

		qualifier = -1;
		if(*format == 'h' || *format == 'l' || *format == 'L' || *format == 'Z' || *format == 'z'){
			qualifier = *format++;
			if(qualifier == *format){
				if(qualifier == 'h'){
					qualifier = 'H';
					format++;
				} else if(qualifier == 'l'){
					qualifier = 'L';
					format++;
				}
			}
		}

		base = 10;
		is_sign = 0;

		if(!*format || !*str){
      break;
    }

		switch(*format++){
      case 'c':
        {
    			char * s = (char *) va_arg(args, char*);
    			if(field_width == -1){
            field_width = 1;
          }
    			do {
    				*s++ = *str++;
    			} while(--field_width > 0 && *str);
    			num++;
  		  }
  		  continue;

  		case 's':
        {
    			char * s = (char *) va_arg(args, char *);
    			if(field_width == -1){
            field_width = __INT_MAX__;
          }
    			/* first, skip leading white space in buffer */
    			while(isspace(*str)){
            str++;
          }
    			/* now copy until next white space */
    			while(*str && !isspace(*str) && field_width--){
    				*s++ = *str++;
    			}
    			*s = '\0';
    			num++;
    		}
    		continue;

  		case 'n':
        {
    			int * i = (int *) va_arg(args, int*);
    			*i = str - buf;
    		}
  		  continue;

  		case 'o':
  			base = 8;
  			break;

  		case 'x':
  		case 'X':
  			base = 16;
  			break;

  		case 'i':
        base = 0;

      case 'd':
  			is_sign = 1;

      case 'u':
  			break;

      case '%':
  			if (*str++ != '%'){
          return num;
        }
  			continue;

      default:
  			return num;
		}

		while(isspace(*str)){
      str++;
    }

		digit = *str;

    if(is_sign && digit == '-'){
      digit = *(str + 1);
    }
		if(!digit || (base == 16 && !isxdigit(digit)) || (base == 10 && !isdigit(digit)) || (base == 8 && (!isdigit(digit) || digit > '7')) || (base == 0 && !isdigit(digit))){
      break;
    }

		switch(qualifier) {
  		case 'H':
  			if(is_sign){
  				signed char * s = (signed char *) va_arg(args, signed char *);
  				*s = (signed char) strtol(str, &next, base);
  			} else {
  				unsigned char * s = (unsigned char *) va_arg(args, unsigned char *);
  				*s = (unsigned char) strtoul(str, &next, base);
  			}
  			break;

  		case 'h':
  			if(is_sign){
  				short * s = (short *) va_arg(args, short *);
  				*s = (short) strtol(str, &next, base);
  			} else {
  				unsigned short * s = (unsigned short *) va_arg(args, unsigned short *);
  				*s = (unsigned short) strtoul(str, &next, base);
  			}
  			break;

  		case 'l':
  			if(is_sign){
  				long * l = (long *) va_arg(args, long *);
  				*l = strtol(str, &next, base);
  			} else {
  				unsigned long * l = (unsigned long *) va_arg(args, unsigned long *);
  				*l = strtoul(str, &next, base);
  			}
  			break;

  		case 'L':
  			if(is_sign){
  				long long * l = (long long *) va_arg(args, long long *);
  				*l = strtoll(str, &next, base);
  			} else {
  				unsigned long long * l = (unsigned long long *) va_arg(args, unsigned long long *);
  				*l = strtoull(str, &next, base);
  			}
  			break;

  		case 'Z':
  		case 'z':
        {
    			size_t * s = (size_t *) va_arg(args, size_t *);
    			*s = (size_t) strtoul(str, &next, base);
    		}
    		break;

  		default:
  			if(is_sign){
  				int * i = (int *) va_arg(args, int *);
  				*i = (int) strtol(str, &next, base);
  			} else {
  				unsigned int * i = (unsigned int *) va_arg(args, unsigned int *);
  				*i = (unsigned int) strtoul(str, &next, base);
  			}
  			break;
		}
		num++;

		if(!next){
      break;
    }
		str = next;
	}

	return num;
}

// writes character to file
int fputc(int character, FILE * file){
  return fwrite((uint8_t *)&character, 1, 1, file);
}

// writes string to file
int fputs(char * str, FILE * file){
  return fwrite(str, strlen(str), 1, file);
}

int getchar(){
  int size = 1;
  while( ioctl(stdin->id, STDIN_CMD_INPUT_READY, (uint32_t *) &size) == 0 ){}
  return fgetc(stdin);
}

char * gets(char * string, int size){
  while( ioctl(stdin->id, STDIN_CMD_INPUT_READY, (uint32_t *) &size) == 0 ){
    sleep(10);
  }
  size = fread(string, size, 1, stdin);
  string[size] = '\0';
  return string;
}

int fgetc(FILE * file){
  char c = (char) -1;
  fread(&c, 1, 1, file);
  return (int) c;
}

// closes file
int fclose(FILE * file){
  int ret = close(file->id);
  free(file);
  return ret;
}

// opens file
FILE * fopen(char * filename, char * mode){
  int mode_num = -1;
  if(!strcmp(mode, "r")){
    mode_num = MODE_R;
  } else if(!strcmp(mode, "w")){
    mode_num = MODE_W;
  } else if(!strcmp(mode, "rw")){
    mode_num = MODE_RW;
  }
  if(mode_num < 0){
    return NULL;
  } else {
    FILE * file = (FILE *) malloc(sizeof(FILE));
    if(file == NULL){
      return NULL;
    }
    file->id = open((char *) filename);
    if(file->id == -1){
      free(file);
      return NULL;
    }
    file->mode = mode_num;
    file->seek_index = 0;
    return file;
  }
  return NULL;
}

// writes size * count bytes into file from ptr (start position determined by fseek)
size_t fwrite(void * ptr, size_t size, size_t count, FILE * file){
  uint32_t total_count = ((uint32_t) size * count);
  size_t ret = (size_t) write(file->id, file->seek_index, total_count, (uint8_t *) ptr);
  file->seek_index += total_count;
  return  ret;
}

// reads size * count bytes into ptr from file (start position determined by fseek)
size_t fread(void * ptr, size_t size, size_t count, FILE * file){
  uint32_t total_count = ((uint32_t) size * count);
  size_t ret = (size_t) read(file->id, file->seek_index, total_count, (uint8_t *) ptr);
  file->seek_index += total_count;
  return  ret;
}

// seeks into the file given an offset from one of the 3 seek positions
// seek set - beginning
// seek cur - current index
// seek end - end of file
int fseek(FILE * file, long offset, int origin){
  if(origin == SEEK_SET){
    file->seek_index = (uint32_t) offset;
    return 0;
  } else if(origin == SEEK_CUR){
    file->seek_index += (uint32_t) offset;
    return 0;
  } else if(origin == SEEK_END){
    return 0;
  }
  return -1;
}

// retrieves current seek index of the file
long ftell(FILE * file){
  return (long) file->seek_index;
}

// !!! unimplemented !!!
void setbuf(FILE * file, char * buffer){
  return;
}

// !!! unimplemented !!!
int fflush(FILE * file){
  return -1;
}
