#ifndef SERIAL_H
#define SERIAL_H

#include <drivers/devfs/devfs.h>

#define SERIAL_PORT 0x3F8

void initialise_serial();

void serial_send(char character);

char serial_receive();

int serial_dev_open(io_node_t * device);

int serial_dev_close(io_node_t * device);

int serial_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int serial_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int serial_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments);

uint32_t serial_dev_status(io_node_t * device);

#endif
