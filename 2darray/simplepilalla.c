#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>

//Compile using: gcc simple.c lodepng.c -Wall -o out

int main(int argc, char *argv[]) {
  const char* filename = "test.png";
  const char* out_filename = "testi.png";

  unsigned char image[];
  unsigned char *ptr = image;
  unsigned width, height;

  //unsigned bitdepth = 4;
  LodePNGColorMode mode_rgba; lodepng_color_mode_init(&mode_rgba); mode_rgba.bitdepth = 16; mode_rgba.colortype = LCT_RGBA;
  LodePNGColorMode mode_grey; lodepng_color_mode_init(&mode_grey); mode_grey.bitdepth = 8; mode_grey.colortype = LCT_GREY;

  unsigned bpp = lodepng_get_bpp(&mode_grey);

  //Decode
  //unsigned error = lodepng_decode_file(&image, &width, &height, filename, LCT_GREY, bitdepth);
  unsigned error = lodepng_decode32_file(&ptr, &width, &height, filename);
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
  printf("%u, %u, %u\n", width, height,((width * height * bpp + 7) / 8));
  unsigned char* image_out = malloc((width * height * bpp + 7) / 8);

  //Convert to greyscale
 // lodepng_convert(image_out, image, &mode_rgba, &mode_grey, width, height);


  //TODO: tässä vielä tehdä tää: thresholds pixels (makes pixels equal to Zero if they are less than 128)
  size_t n = (sizeof(image) / sizeof(image[0]));
  printf("%ld\n", n);
  for (size_t i = 0; i <= n; i++) {
      printf("%u\n", image_out[i]);
      if (image_out[i] < 128) {
          image_out[i] = 0;
      }
  }
  //Encode
  //error = lodepng_encode_file(out_filename, image, width, height, LCT_GREY, bitdepth);
  error = lodepng_encode32_file(out_filename, image_out, width, height);
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

  free(ptr);
  free(image_out);

  return 0;
}
