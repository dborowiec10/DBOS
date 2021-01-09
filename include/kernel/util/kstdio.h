#ifndef KSTDIO_H
#define KSTDIO_H

#include <stdarg.h>

// memory functions
void memset_8(uint8_t * dest, uint8_t val, size_t len);

void memset_16(uint16_t * dest, uint16_t val, size_t len);

void memset_32(uint32_t * dest, uint32_t val, size_t len);

void memcpy_8(uint8_t * dest, uint8_t * src, size_t len);

void memcpy_16(uint16_t * dest, uint16_t * src, size_t len);

void memcpy_32(uint32_t * dest, uint32_t * src, size_t len);

// port input and output
void read_port_32_mult(uint32_t port, uint32_t * buffer, int count);

void read_port_16_mult(uint16_t port, uint8_t * buffer, uint32_t size);

void write_port_16_mult(uint16_t port, uint8_t * buffer, uint32_t size);

uint8_t read_port_8(uint16_t port);

void write_port_8(uint16_t port, uint8_t data);

uint32_t read_port_32(uint16_t port);

void write_port_32(uint16_t port, uint32_t data);

uint16_t read_port_16(uint16_t port);

void write_port_16(uint16_t port, uint16_t data);

// writing to buffer
void ksprintf(char * dest, const char * text, ...);

int kputhexbufat(uint32_t hex, char * buffer, int at);

int kputuintbufat(uint32_t uinteg, char * buffer, int at);

int kputintbufat(int num, char * buffer, int at);

void kputcharatbuf(char c, char * buffer, int at);

// writing to screen
void kprintf(char * string, ...);

void kputchar(char c);

void kputhex(uint32_t hex);

void kputint(int num);

void kputuint(uint32_t uinteg);

void kstdio_base_init();


#endif
