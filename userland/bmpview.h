#ifndef BMPVIEW_H
#define BMPVIEW_H
#include <stdint.h>
#include <stddef.h>
#include <libbmp.h>

typedef struct image {
  bmp_img_t * image;
  uint32_t * image_buffer;
} image_t;


#endif
