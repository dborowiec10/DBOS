#include <stddef.h>
#include <stdint.h>
#include <kernel/util/string.h>
#include <kernel/util/kstdio.h>
#include <kernel/util/logging.h>
#include <kernel/vfs/vfs.h>
#include <drivers/devfs/devfs.h>
#include <drivers/devices/serial/serial.h>

vfs_gen_node_t * serial_dev_file = NULL;

int ser_dev_ready = 0;

// logs a string to the serial output
void log(char * string){
  if(string != NULL){
    int len = strlen(string);
    for(int i = 0; i < len; i++){
      logchar(string[i]);
      if(string[i] == '\n'){
        logchar('\r');
      }
    }
  }
}

// logs a character to the serial output
void logchar(char c){
  if(serial_dev_file == NULL && vfs_is_ready() && devfs_is_ready()){
    serial_dev_file = vfs_find_node("/dev/ser");
    if(serial_dev_file != NULL){
      ser_dev_ready = 1;
    }
  }
  if(ser_dev_ready){
    serial_dev_file->node_write(serial_dev_file, 0, 1, (uint8_t *) &c);
  } else {
    serial_send(c);
  }
  if(c == '\n'){
    logchar('\r');
  }
}
