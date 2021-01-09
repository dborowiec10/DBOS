#include <stdint.h>
#include <stddef.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/logging.h>
#include <kernel/vfs/vfs.h>

vfs_gen_node_t * console = NULL;

int base_init = 0;

void kstdio_base_init(){
  base_init = 1;
}

// prints single character to display
void kputchar(char c){
  if(base_init != 0){
    if(console == NULL){
      console = vfs_find_node("/dev/gtty");
    }
    if(console != NULL && console->node_status(console) != 0){
      console->node_write(console, 0xFFFFFFFF, 1, (uint8_t *)&c);
    } else {
      logchar(c);
    }
  } else {
    logchar(c);
  }
}

// inserts formatted string with arguments to a provided dest buffer
void ksprintf(char * dest, const char * text, ...){
  char * dest_save_addr = dest;
  int i = 0;
  int count = 0;
  va_list args;
  va_start(args, text);

  char * str_temp;
  char c_temp;
  int dec_temp;
  uint32_t uns_temp;

  while(text[i]){
    if(text[i] == '%'){
      i++;
      switch (text[i]) {
        case 's':
          str_temp = va_arg(args, char *);
          int j = 0;
          while(str_temp[j]){
            kputcharatbuf(str_temp[j++], dest_save_addr, count);
            count++;
          }
          break;
        case 'c':
          c_temp = (char) va_arg(args, int);
          kputcharatbuf(c_temp, dest_save_addr, count);
          count++;
          break;
        case 'd':
          dec_temp = va_arg(args, int);
          count += (kputintbufat(dec_temp, dest_save_addr, count));
          break;
        case 'u':
          uns_temp = va_arg(args, uint32_t);
          count += (kputuintbufat(uns_temp, dest_save_addr, count));
          break;
        case 'x':
          uns_temp = va_arg(args, uint32_t);
          count += (kputhexbufat(uns_temp, dest_save_addr, count));
          break;
        default:
          kputcharatbuf(text[i], dest_save_addr, count++);
      }
      i++;
    } else {
      kputcharatbuf(text[i], dest_save_addr, count++);
      i++;
    }
  }
  dest_save_addr[count] = '\0';
  va_end(args);
}

// prints formatted string to the screen
void kprintf(char * string, ...){
  int i = 0;
  va_list args;
  va_start(args, string);

  char * str_temp;
  char c_temp;
  int dec_temp;
  uint32_t uns_temp;

  while(string[i]){
    if(string[i] == '%'){
      i++;
      switch (string[i]){
        case 's':
          str_temp = va_arg(args, char *);
          int j = 0;
          while(str_temp[j]){
            kputchar(str_temp[j++]);
          }
          break;
        case 'c':
          c_temp = (char) va_arg(args, int);
          kputchar(c_temp);
          break;
        case 'd':
          dec_temp = va_arg(args, int);
          kputint(dec_temp);
          break;
        case 'u':
          uns_temp = va_arg(args, uint32_t);
          kputuint(uns_temp);
          break;
        case 'x':
          uns_temp = va_arg(args, uint32_t);
          kputhex(uns_temp);
          break;
        default:
          kputchar(string[i]);
      }
      i++;
    } else {
      kputchar(string[i]);
      i++;
    }
  }
  va_end(args);
}

// puts signed decimal integer to the screen
void kputint(int num){
  if(num >= 0){
    kputuint((uint32_t) num);
  } else {
    kputchar('-');
    kputuint((uint32_t) -num);
  }
}

// puts unsigned integer to the screen
void kputuint(uint32_t uinteg){
  uint32_t num;
  uint32_t divisor = 1000000000;
  uint8_t unsigned_string[255];
  uint32_t index = 0;
  while( (uinteg / divisor == 0) && (divisor >= 10) ){
    divisor /= 10;
  }
  num = uinteg;
  while( divisor >= 10){
    unsigned_string[index++] = ( (uint8_t) ( (int) '0' + num / divisor) );
    num = num % divisor;
    divisor /= 10;
  }
  unsigned_string[index++] = (uint8_t) ( (int) '0' + num);
  unsigned_string[index] = 0;
  num = 0;
  while(unsigned_string[num]){
    kputchar(unsigned_string[num++]);
  }
}

// puts hexadecimal representation of a provided integer to the screen
void kputhex(uint32_t hex){
  kputchar('0');
  kputchar('x');
  uint8_t no_ze = 1;
  int i = 28;
  signed int temp;
  for (; i > 0; i -= 4){
      temp = (hex >> i) & 0xF;
      if ( (temp == 0) && (no_ze != 0) ){
          continue;
      }
      if (temp >= 0xA){
          no_ze = 0;
          kputchar(temp - 0xA + 'A');
      } else {
          no_ze = 0;
          kputchar(temp + '0');
      }
  }
  temp = hex & 0xF;
  if (temp >= 0xA){
      kputchar(temp - 0xA + 'A');
  } else {
      kputchar(temp + '0');
  }
}

// returns number of written characters to buffer
int kputhexbufat(uint32_t hex, char * buffer, int at){
  int c = 0;
  buffer[at] = '0';
  at++;
  c++;
  buffer[at] = 'x';
  c++;
  uint8_t no_ze = 1;
  int i = 28;
  signed int temp;
  for (; i > 0; i -= 4){
      temp = (hex >> i) & 0xF;
      if ( (temp == 0) && (no_ze != 0) ){
          continue;
      }
      if (temp >= 0xA){
          no_ze = 0;
          at++;
          c++;
          kputcharatbuf((char)(temp - 0xA + 'A'), buffer, at);
      } else {
          no_ze = 0;
          at++;
          c++;
          kputcharatbuf((char)(temp + '0'), buffer, at);
      }
  }
  temp = hex & 0xF;
  if (temp >= 0xA){
      at++;
      c++;
      kputcharatbuf((char)(temp - 0xA + 'A'), buffer, at);
  } else {
      at++;
      c++;
      kputcharatbuf((char)(temp + '0'), buffer, at);
  }
  return c;
}

// puts unsigned integer at buffer @ at index
int kputuintbufat(uint32_t uinteg, char * buffer, int at){
  char * buf_save = buffer;
  int c = 0;
  uint32_t num;
  uint32_t divisor = 1000000000;
  uint8_t unsigned_string[255];
  uint32_t index = 0;
  while( (uinteg / divisor == 0) && (divisor >= 10) ){
    divisor /= 10;
  }
  num = uinteg;
  while( divisor >= 10){
    unsigned_string[index++] = ( (uint8_t) ( (int) '0' + num / divisor) );
    num = num % divisor;
    divisor /= 10;
  }
  unsigned_string[index++] = (uint8_t) ( (int) '0' + num);
  unsigned_string[index] = 0;
  num = 0;
  while(unsigned_string[num]){
    kputcharatbuf((char) unsigned_string[num++], buf_save, at);
    at++;
    c++;
  }
  return c;
}

// puts integer at buffer @ at index
int kputintbufat(int num, char * buffer, int at){
  int c = 0;
  char * buf_save = buffer;
  if(num >= 0){
    c += kputuintbufat((uint32_t) num, buf_save, at);
  } else {
    kputcharatbuf('-', buf_save, at);
    c++;
    c += kputuintbufat((uint32_t) -num, buf_save, at+1);
  }
  return c;
}

// puts character at buffer @ at index
void kputcharatbuf(char c, char * buffer, int at){
  buffer[at] = c;
}

// sets one byte [val] into dest len times
void memset_8(uint8_t * dest, uint8_t val, size_t len){
  size_t i = 0;
  while(i < len){
    dest[i] = val;
    i++;
  }
}

// sets 2 byte [val] into dest len times
void memset_16(uint16_t * dest, uint16_t val, size_t len){
  size_t i = 0;
  while(i < len){
    dest[i] = val;
    i++;
  }
}

// sets 4 byte [val] into dest len times
void memset_32(uint32_t * dest, uint32_t val, size_t len){
  size_t i = 0;
  while(i < len){
    dest[i] = val;
    i++;
  }
}

// copies len bytes of memory from src to dest
void memcpy_8(uint8_t * dest, uint8_t * src, size_t len){
  size_t i = 0;
  while(i < len){
    dest[i] = src[i];
    i++;
  }
}

// copies len words of memory from src to dest
void memcpy_16(uint16_t * dest, uint16_t * src, size_t len){
  size_t i = 0;
  while(i < len){
    dest[i] = src[i];
    i++;
  }
}

// copies len double words of memory from src to dest
void memcpy_32(uint32_t * dest, uint32_t * src, size_t len){
  size_t i = 0;
  while(i < len){
    dest[i] = src[i];
    i++;
  }
}


// reads a byte from port
uint8_t read_port_8(uint16_t port){
  uint8_t rv;
  __asm__ __volatile__("inb %1, %0"
                       : "=a" (rv)
                       :"dN" (port)
  );
  return rv;
}

// writes a byte to port
void write_port_8(uint16_t port, uint8_t data){
  __asm__ __volatile__("outb %1, %0"
                       :
                       : "dN" (port), "a" (data)
  );
}

// reads 2 bytes from port
uint16_t read_port_16(uint16_t port){
	uint16_t rv;
	__asm__ __volatile__("inw %1, %0"
                       : "=a" (rv)
                       : "dN" (port)
  );
	return rv;
}

// writes 2 bytes to port
void write_port_16(uint16_t port, uint16_t data){
	__asm__ __volatile__("outw %1, %0"
                       :
                       : "dN" (port), "a" (data)
  );
}

// reads 4 bytes from port
uint32_t read_port_32(uint16_t port){
	uint32_t rv;
	__asm__ __volatile__("inl %%dx, %%eax"
                       : "=a" (rv)
                       : "dN" (port)
  );
	return rv;
}

// writes 4 bytes to port
void write_port_32(uint16_t port, uint32_t data){
	__asm__ __volatile__("outl %%eax, %%dx"
                       :
                       : "dN" (port), "a" (data)
  );
}

// reads count number of 32bit words to buffer from port
void read_port_32_mult(uint32_t port, uint32_t * buffer, int count){
  __asm__ __volatile__("cld; rep; insl"
                       :
                       : "D" (buffer), "d" (port), "c" (count)
  );
}

// reads size number of 16bit words to buffer from port
void read_port_16_mult(uint16_t port, uint8_t * buffer, uint32_t size){
	__asm__ __volatile__("rep insw"
                       : "+D" (buffer), "+c" (size)
                       : "d" (port)
                       : "memory"
  );
}

// writes size number of 16bit words to port from buffer
void write_port_16_mult(uint16_t port, uint8_t * buffer, uint32_t size){
	__asm__ __volatile__("rep outsw"
                       : "+S" (buffer), "+c" (size)
                       : "d" (port)
  );
}
