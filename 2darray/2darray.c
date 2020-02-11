#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>

//Compile using: gcc simple.c lodepng.c -Wall -o out

int main(int argc, char *argv[]) {
  const char* filename = "test.png";
  const char* out_filename = "testi.png";

  unsigned char* image = 0;
  unsigned width, height;

  unsigned bitdepth = 8;

  //Decode
  unsigned error = lodepng_decode_file(&image, &width, &height, filename, LCT_GREY, bitdepth);
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));


  //threshold
  for (int i = 0; i <= (width * height); i++) {
      //printf("%u\n", image[i]);
      if (image[i] < 128) {
          image[i] = 0;
      }
  }

  //Encode
  error = lodepng_encode_file(out_filename, image, width, height, LCT_GREY, bitdepth);
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

  free(image);

  return 0;
}
