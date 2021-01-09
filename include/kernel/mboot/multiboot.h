#ifndef MULTIBOOT_H
#define MULTIBOOT_H
#include <stdint.h>
#include <stddef.h>

#define MAGIC 0x2BADB002

struct kernel_symbol_table {
  uint32_t tab_size;
  uint32_t str_size;
  uint32_t address_of_size;
  uint32_t reserved;
};

struct section_header_table {
  uint32_t number;
  uint32_t size;
  uint32_t address;
  uint32_t shndx;
};

typedef struct vbe_info_control {
  char vbe_signature[4];
  uint16_t vbe_version;
  uint32_t vbe_oem_string_pointer;
  uint32_t vbe_capabilities;
  uint32_t vbe_video_mode_pointer;
  uint16_t vbe_total_memory_blocks;
} vbe_control_info_t;


typedef struct {
	uint16_t mode_attr;
  uint8_t window_a_attr;
  uint8_t window_b_attr;
	uint16_t granularity;
	uint16_t winsize;
  uint16_t win_a_segment_start;
  uint16_t win_b_segment_start;
	uint32_t window_real_mode_function_pointer;
	uint16_t bytes_per_scan_line;
  uint16_t x_resolution;
  uint16_t y_resolution;
  uint8_t x_char_size;
  uint8_t y_char_size;
  uint8_t no_of_planes;
  uint8_t bits_per_pixel;
  uint8_t no_of_banks;
  uint8_t memory_model;
  uint8_t bank_size;
  uint8_t no_of_image_pages;
  uint8_t res0;

	uint8_t red_mask_size;
  uint8_t red_field_position;
	uint8_t green_mask_size;
  uint8_t green_field_position;
	uint8_t blue_mask_size;
  uint8_t blue_field_position;
	uint8_t reserved_mask_size;
  uint8_t reserved_field_position;
	uint8_t direct_color_mode_attributes;

	uint32_t linear_framebuffer_physical_base;
	uint32_t res1;
	uint16_t res2;
} __attribute__ ((packed)) vbe_mode_info_t;

typedef struct multiboot_information {
  uint32_t flags;
  uint32_t memory_lower;
  uint32_t memory_upper;
  uint32_t boot_device;
  uint32_t cmd;
  uint32_t boot_modules_count;
  uint32_t * boot_modules_address;
  union {
    struct kernel_symbol_table kst;
    struct section_header_table sht;
  } unio;
  uint32_t memory_map_lenght;
  uint32_t memory_map_address;
  uint32_t drives_length;
  uint32_t drives_address;
  uint32_t rom_config_table;
  uint32_t * bootloader;
  uint32_t apm_table;

  vbe_control_info_t * vbe_control_info;
  vbe_mode_info_t * vbe_mode_info;
  uint16_t vbe_video_mode;
  uint16_t vbe_video_interface_seg;
  uint16_t vbe_video_interface_offset;
  uint16_t vbe_video_interface_length;

  uint64_t framebuffer_addr;
  uint32_t framebuffer_pitch;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint32_t framebuffer_bpp;
  uint32_t framebuffer_type;
} multiboot_info_t;

typedef struct param {
  char * param_key;
  char * param_val;
} mboot_param_t;


void parse_multiboot_params(multiboot_info_t * mboot);

char * params_find_by_key(char * key);

#endif
