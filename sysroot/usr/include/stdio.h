#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STDOUT_CMD_SET_MODE         0x01
#define STDOUT_CMD_GET_MODE         0x02

#define STDOUT_CMD_GETW             0x03
#define STDOUT_CMD_GETH             0x04

#define STDOUT_CMD_CLEAR            0x05

#define STDOUT_CMD_GET_FG_COLOUR    0x06
#define STDOUT_CMD_SET_FG_COLOUR    0x07

#define STDOUT_CMD_GET_BG_COLOUR    0x08
#define STDOUT_CMD_SET_BG_COLOUR    0x09

#define STDOUT_CMD_ECHO_ENABLE      0x0A
#define STDOUT_CMD_ECHO_DISABLE     0x0B

#define STDIN_CMD_INPUT_READY       0x0C

#define STDOUT_CMD_GET_DISP_TYPE    0x0D

#define STDOUT_MODE_TXT         0x00
#define STDOUT_MODE_PX          0x01

#define STDOUT_DISP_TYPE_VGA    0x00
#define STDOUT_DISP_TYPE_VESA   0x01

#define STDOUT_ID 0

#define STDIN_ID 0

#define STDERR_ID 0

#define SEEK_SET 0

#define SEEK_CUR 1

#define SEEK_END 2

#define MODE_R 0

#define MODE_W 1

#define MODE_RW 2

#define NULL ((void *) 0)

#define EOF -1

#define FILE_TYPE_SOCK 1
#define FILE_TYPE_SLNK 2
#define FILE_TYPE_FILE 4
#define FILE_TYPE_BLKD 8
#define FILE_TYPE_CHRD 16
#define FILE_TYPE_DIRE 32
#define FILE_TYPE_FIFO 64
#define FILE_TYPE_SDIR 128

typedef struct {
  int id;
  uint32_t seek_index;
  int mode;
} FILE;

extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;

void stdio_init(void);

int fclose(FILE *);

int fflush(FILE *);

FILE * fopen(char *, char *);

int fprintf(FILE *, char *, ...);

size_t fread(void *, size_t, size_t, FILE *);

int fseek(FILE *, long, int);

long ftell(FILE *);

size_t fwrite(void *, size_t, size_t, FILE *);

void setbuf(FILE *, char *);


int vfprintf(FILE *, char *, va_list);

int vsprintf(char * str, char * format, va_list arg);

int vsnprintf(char * str, size_t size, char * format, va_list arg);

int snprintf(char * str, size_t size, char * format, ...);

int sprintf(char * str, char * format, ...);

int fprintf(FILE * file, char * format, ...);

int vsscanf(char * str, char * format, va_list args);

int sscanf(char * buf, char * fmt, ...);

int fputc(int character, FILE * file);

int fputs(char * str, FILE * file);

int fgetc(FILE * file);

int getchar();

char * gets(char * string, int size);

int printf(char *, ...);

#ifdef __cplusplus
}
#endif

#endif
