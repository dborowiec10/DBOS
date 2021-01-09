#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <libbmp.h>

// loads a bmp image given a path, returns a pointer to it
bmp_img_t * bmp_load(char * path){
  bmp_img_t * image = NULL;
  // open the file
  FILE * image_file = fopen(path, "r");
  if(image_file == NULL){
    return NULL;
  }
  // fetch the header
  bmp_hdr_t * header = (bmp_hdr_t *) malloc(sizeof(bmp_hdr_t));
  fseek(image_file, 0, SEEK_SET);
  size_t read = fread(header, 14, 1, image_file);
  // if didnt read exactly 14 bytes
  if(read != 14){
    free(header);
    fclose(image_file);
    return NULL;
  }

  // fetch info header
  bmp_info_hdr_t * info_header = (bmp_info_hdr_t *) malloc(sizeof(bmp_info_hdr_t));
  read = fread(info_header, 40, 1, image_file);
  // if didnt read exactly 40 bytes
  if(read != 40){
    free(header);
    free(info_header);
    fclose(image_file);
    return NULL;
  }
  // allocate space for structures
  image = (bmp_img_t *) malloc(sizeof(bmp_img_t));

  // insert data
  image->header = header;
  image->info_header = info_header;
  image->image_file = image_file;

  // return image
  return image;
}

// retrieves image size in bytes
uint32_t bmp_size(bmp_img_t * image){
  return (uint32_t)(image->info_header->width * image->info_header->height) * 3;
}

// retrieves suspected image size in bytes with prepended alpha channel
uint32_t bmp_size_with_alpha(bmp_img_t * image){
  return (uint32_t)(bmp_pixels(image) * 4);
}

// retrieves no of pixels in the image
uint32_t bmp_pixels(bmp_img_t * image){
  return (uint32_t)(bmp_size(image) / 3);
}

// retrieves width of bitmap in pixels
uint32_t bmp_width(bmp_img_t * image){
  return (uint32_t) image->info_header->width;
}

// retrieves height of bitmap in pixels
uint32_t bmp_height(bmp_img_t * image){
  return (uint32_t) image->info_header->height;
}

// reads size pixels starting from start from image into buffer, combining each with alpha
int bmp_read(bmp_img_t * image, uint32_t * buffer, uint32_t start, uint32_t pixels, uint8_t alpha){
  FILE * file = image->image_file;
  if(file == NULL){
    return -1;
  }
  fseek(file, image->header->offset + start, SEEK_SET);

  bmp_pix_t * pixs = (bmp_pix_t *) malloc(sizeof(bmp_pix_t) * pixels);

  size_t read = fread(pixs, pixels * sizeof(bmp_pix_t), 1, file);

  if(read != (size_t)(pixels * 3)){
    return -1;
  }

  for(int i = 0; i < image->info_header->width; i++){
    // bmp images read from bottom to top, thus invert reads
    for(int j = 0, inv_j = (image->info_header->height - 1); j < image->info_header->height; j++, inv_j--){
      int reg_index = (int)(i + (j * image->info_header->width));
      int inv = (int)(i + (inv_j * image->info_header->width));
      buffer[reg_index] = 0xFFFFFFFF & ((uint32_t)((alpha << 24) | (pixs[inv].red << 16) | (pixs[inv].green << 8) | pixs[inv].blue));

    }
  }

  free(pixs);

  return (read / 3) * 4 ;
}

// destroys bmp image structure
void bmp_destroy(bmp_img_t * image){
  free(image->header);
  free(image->info_header);
  fclose(image->image_file);
  free(image);
}
