#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <string.h>
#include <libbmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <liblist.h>
#include "bmpview.h"

list_t * images = NULL;

int display_width = 0;

int display_height = 0;

uint32_t colour;

uint32_t save_colour;

#define COL_VGA_RED		   0x04

void image_display(image_t * image);

// program used to display bitmap images
// image file paths should be provided in full as the parameters
// each image file path should be specified separately
// program will display images on a loop with a delay 3 times and then exit
int main(int argc, char * argv[]){

  // check if display supports pixel mode
  int type = ioctl(stdout->id, STDOUT_CMD_GET_DISP_TYPE, NULL);
  if(type == STDOUT_DISP_TYPE_VGA){
    colour = COL_VGA_RED;
    save_colour = (uint32_t) ioctl(stdout->id, STDOUT_CMD_GET_FG_COLOUR, NULL);
    ioctl(stdout->id, STDOUT_CMD_SET_FG_COLOUR, &colour);
    printf("%s", "ERROR: Current display capabilities do not allow for displaying images!");
    ioctl(stdout->id, STDOUT_CMD_SET_FG_COLOUR, &save_colour);
    abort();
  }

  // create a list for images
  images = list_create();

  for(int i = 0; i < argc; i++){
    // for each image, attempt to load it
    bmp_img_t * image = bmp_load(argv[i]);
    if(image != NULL){
      // allocate a buffer for it
      uint32_t * buffer = (uint32_t *) malloc( sizeof(uint32_t) * bmp_width(image) * bmp_height(image) );

      uint8_t alpha = 0x00;

      // read in the data
      bmp_read(image, buffer, 0, bmp_pixels(image), alpha);

      // allocate a structure for it
      image_t * img = (image_t *) malloc(sizeof(image_t));

      img->image = image;
      img->image_buffer = buffer;

      // add image to list
      list_append(images, (uint32_t *) img);
    } else {
      colour = 0x00FF0000;
      save_colour = 0x00FFFFFF;
      ioctl(stdout->id, STDOUT_CMD_SET_FG_COLOUR, &colour);
      printf("%s", "ERROR: One or more image files could not be loaded!\n");
      ioctl(stdout->id, STDOUT_CMD_SET_FG_COLOUR, &save_colour);
      abort();
    }
  }

  // reset stdout mode to pixels
  uint32_t mode = STDOUT_MODE_PX;

  ioctl(stdout->id, STDOUT_CMD_SET_MODE, &mode);

  // fetch pixel dimensions of the display
  display_width = ioctl(stdout->id, STDOUT_CMD_GETW, NULL);

  display_height = ioctl(stdout->id, STDOUT_CMD_GETH, NULL);

  // clear it out
  ioctl(stdout->id, STDOUT_CMD_CLEAR, NULL);

  // on a loop for 3 times
  int i = 0;
  while(i < 3){
    // for each image
    list_each(item, images){
      image_t * image = (image_t *) item->data;
      // display the image
      image_display(image);
      // sleep for 20000 timer ticks
      sleep(20000);
      // clear the display
      ioctl(stdout->id, STDOUT_CMD_CLEAR, NULL);
    }
    i++;
  }
}

// displays a bmp image
void image_display(image_t * image){
  // fetch image and its buffer
  bmp_img_t * img = image->image;
  uint32_t * buffer = image->image_buffer;

  // fetch image width and height
  int img_width = bmp_width(img);
  int img_height = bmp_height(img);

  // calculate margins to center the image on the display
  int margin_horiz = (display_width - img_width);
  int margin_vert = (display_height - img_height);

  // divide margins if they are < 0
  if(margin_horiz > 0){
    margin_horiz /= 2;
  }
  if(margin_vert > 0){
    margin_vert /= 2;
  }

  // seek to the top left corner of the image
  fseek(stdout, margin_vert * display_width + margin_horiz, SEEK_SET);

  // for each row in image
  for(int i = 0; i < img_height; i++){

    // write buffer contents to screen for length of an image width
    fwrite(&buffer[i * img_width], (uint32_t) img_width, 1, stdout);

    // seek to the next row
    fseek(stdout, margin_horiz + margin_horiz, SEEK_CUR);

  }

}
