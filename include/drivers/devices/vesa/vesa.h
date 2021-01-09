#ifndef VESA_H
#define VESA_H

#include <drivers/devfs/devfs.h>

#define VESA_PCI_VENDOR_ID                0x1234
#define VESA_PCI_DEVICE_ID                0x1111

#define VESA_PCI_VENDOR_ID_2              0x80EE
#define VESA_PCI_DEVICE_ID_2              0xBEEF

#define VESA_INDEX_PORT                   0x01CE
#define VESA_DATA_PORT                    0x01CF

#define VESA_INDEX_ID                     0x0
#define VESA_INDEX_XRES                   0x1
#define VESA_INDEX_YRES                   0x2
#define VESA_INDEX_BPP                    0x3
#define VESA_INDEX_ENABLE                 0x4
#define VESA_INDEX_BANK                   0x5
#define VESA_INDEX_VIRT_WIDTH             0x6
#define VESA_INDEX_VIRT_HEIGHT            0x7
#define VESA_INDEX_X_OFFSET               0x8
#define VESA_INDEX_Y_OFFSET               0x9
#define VESA_INDEX_VIDEO_MEMORY_64K       0xA

#define VESA_BOCHS_VER_OLDEST             0xB0C0
#define VESA_BOCHS_VER_NEWEST             0xB0C5

#define VESA_BPP_4                        0x04
#define VESA_BPP_8                        0x08
#define VESA_BPP_15                       0x0F
#define VESA_BPP_16                       0x10
#define VESA_BPP_24                       0x18
#define VESA_BPP_32                       0x20

#define VESA_DISPLAY_DISABLED             0x00
#define VESA_DISPLAY_ENABLED              0x01
#define VESA_DISPLAY_GETCAPS              0x02
#define VESA_DISPLAY_8BIT_DAC             0x20
#define VESA_DISPLAY_LFB_ENABLED          0x40
#define VESA_DISPLAY_NOCLEARMEM           0x80

#define VESA_CMD_CLRSC                    0x01
#define VESA_CMD_DRAWPX                   0x02
#define VESA_CMD_GETPX                    0x03
#define VESA_CMD_GETW                     0x04
#define VESA_CMD_GETH                     0x05
#define VESA_CMD_MOVD                     0x06
#define VESA_CMD_SETD                     0x07
#define VESA_CMD_CLRFROM                  0x08

void initialise_vesa(multiboot_info_t * mboot);

int vesa_dev_open(io_node_t * device);

int vesa_dev_close(io_node_t * device);

int vesa_dev_read(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int vesa_dev_write(io_node_t * device, uint32_t start_at, uint32_t size, uint8_t * buffer);

int vesa_dev_control(io_node_t * device, uint32_t command, uint32_t * arguments);

uint32_t vesa_dev_status(io_node_t * device);

int vesa_draw_pixel(int x, int y, uint32_t pixel);

uint32_t vesa_get_pixel_value(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);

uint32_t vesa_get_pixel_at(int x, int y);

void vesa_clear_screen();

void vesa_clear_from(uint32_t location_index);

void vesa_move_data(int to, int from, int size);

void vesa_set_data(int begin, int length, uint32_t data);

#endif
