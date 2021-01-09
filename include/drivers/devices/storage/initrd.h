#ifndef INITRD_H
#define INITRD_H
#include <stdint.h>
#include <stddef.h>
#include <kernel/vfs/vfs.h>

void initialise_initrd();

int initrd_dev_open(io_node_t * device);

int initrd_dev_close(io_node_t * device);

int initrd_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int initrd_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int initrd_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments);

uint32_t initrd_dev_status(io_node_t * device);

#endif
