#include <stddef.h>
#include <stdint.h>
#include <kernel/util/kstdio.h>
#include <kernel/vfs/vfs.h>
#include <kernel/memory/memory_heap.h>
#include <drivers/devices/serial/serial.h>
#include <drivers/devfs/devfs.h>

io_node_t * serial_device = NULL;

vfs_gen_node_t * serial_device_file = NULL;

int serial_status = 0;

int serial_initialised = 0;

// initialises serial device hardware
void serial_init(){
  write_port_8(SERIAL_PORT + 3, 0x80);
  write_port_8(SERIAL_PORT + 0, 0x01);
  write_port_8(SERIAL_PORT + 1, 0x00);
  write_port_8(SERIAL_PORT + 3, 0x03);
  write_port_8(SERIAL_PORT + 2, 0xC7);
  write_port_8(SERIAL_PORT + 4, 0x0B);
  serial_initialised = 1;
}

// initialises serial device
void initialise_serial(){
  // create device node
  serial_device = (io_node_t *) kern_malloc(sizeof(io_node_t));
  serial_device->node_no = get_device_node_no();
  serial_device->node_type = DEVFS_CHAR_DEV;
  serial_device->node_name = "ser\0";
  serial_device->node_size = 0;
  serial_device->node_open = (io_open_t) &serial_dev_open;
  serial_device->node_close = (io_close_t) &serial_dev_close;
  serial_device->node_read = (io_read_t) &serial_dev_read;
  serial_device->node_write = (io_write_t) &serial_dev_write;
  serial_device->node_control = (io_ctrl_t) &serial_dev_control;
  serial_device->node_status = (io_status_t) &serial_dev_status;
  serial_device->base_pointer = NULL;

  vfs_gen_node_t * devfs = vfs_find_node("/dev");
  // add device to the device filesystem
  serial_device_file = create_node_vfs(devfs, serial_device->node_name, VFS_NODE_TYPE_CHRD, (uint32_t *) serial_device);

  serial_status = 1;
}

// opens serial device
int serial_dev_open(io_node_t * device){
  return 0;
}

// closes serial device
int serial_dev_close(io_node_t * device){
  return 0;
}

// reads from serial_device
int serial_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  for(int i = 0; i < (int) size; i++){
    buffer[i] = (uint8_t) serial_receive();
  }
  return size;
}

// writes to serial device
int serial_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer){
  for(int i = 0; i < (int) size; i++){
    serial_send((char) buffer[i]);
  }
  return size;
}

// controls serial device
int serial_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments){
  return 0;
}

// returns status of serial device
uint32_t serial_dev_status(io_node_t * device){
  if(device == serial_device){
    return (uint32_t) serial_status;
  } else {
    return 0xFFFFFFFF;
  }
}

// checks if serial device hardware is ready to continue writing
uint8_t check_if_ready(){
  while((read_port_8(SERIAL_PORT + 5) & 0x20) == 0);
  return 1;
}

// send a single character to the serial device
void serial_send(char character){
  if(!serial_initialised){
    serial_init();
  }
  if(check_if_ready() == 1){
    write_port_8(SERIAL_PORT, character);
  }
}

// check is serial is busy or waiting for input
uint8_t serial_got_something(){
  while( (read_port_8(SERIAL_PORT + 5) & 1) == 0);
  return 1;
}

// receive a single character from serial device
char serial_receive(){
  if(serial_got_something() == 1){
    return read_port_8(SERIAL_PORT);
  } else {
    return '\0';
  }
}
