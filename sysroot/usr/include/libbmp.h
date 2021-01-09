#ifndef LIBBMP_H
#define LIBBMP_H

#include <stdint.h>

typedef struct bmp_header {
  char magic[2];
  uint32_t size;
  uint16_t r1;
  uint16_t r2;
  uint32_t offset;
} __attribute__((packed)) bmp_hdr_t;

typedef struct bmp_info_header {
  uint32_t hdr_size;
  int width;
  int height;
  uint16_t colour_planes;
  uint16_t bpp;
  uint32_t compression;
  uint32_t img_size;
  int x_res;
  int y_res;
  uint32_t no_colours;
  uint32_t important_colours;
} __attribute__((packed)) bmp_info_hdr_t;

typedef struct bmp_pixel {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} __attribute__((packed)) bmp_pix_t;

typedef struct bmp_image {
  bmp_hdr_t * header;
  bmp_info_hdr_t * info_header;
  FILE * image_file;
} bmp_img_t;

bmp_img_t * bmp_load(char * path);

uint32_t bmp_size(bmp_img_t * image);

uint32_t bmp_size_with_alpha(bmp_img_t * image);

uint32_t bmp_pixels(bmp_img_t * image);

uint32_t bmp_width(bmp_img_t * image);

uint32_t bmp_height(bmp_img_t * image);

void bmp_destroy(bmp_img_t * image);

int bmp_read(bmp_img_t * image, uint32_t * buffer, uint32_t start, uint32_t pixels, uint8_t alpha);



#endif
