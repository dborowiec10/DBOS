#ifndef GTTY_H
#define GTTY_H

#include <drivers/devfs/devfs.h>
#include <kernel/process/process.h>

#define GTTY_CMD_SET_MODE       0x01
#define GTTY_CMD_GET_MODE       0x02
#define GTTY_CMD_GETW           0x03
#define GTTY_CMD_GETH           0x04
#define GTTY_CMD_CLEAR          0x05
#define GTTY_CMD_GET_FG_COLOUR  0x06
#define GTTY_CMD_SET_FG_COLOUR  0x07
#define GTTY_CMD_GET_BG_COLOUR  0x08
#define GTTY_CMD_SET_BG_COLOUR  0x09
#define GTTY_CMD_ECHO_ENABLE    0x0A
#define GTTY_CMD_ECHO_DISABLE   0x0B
#define GTTY_CMD_INPUT_READY    0x0C
#define GTTY_CMD_GET_DISP_TYPE  0x0D

#define GTTY_MODE_TXT           0x00
#define GTTY_MODE_PX            0x01

#define GTTY_DISP_TYPE_VGA      0x00
#define GTTY_DISP_TYPE_VESA     0x01

#define GTTY_INPUT_BUFFER_LEN   1024

void initialise_gtty();

int gtty_dev_open(io_node_t * device);

int gtty_dev_close(io_node_t * device);

int gtty_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int gtty_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int gtty_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments);

uint32_t gtty_dev_status(io_node_t * device);



#endif
