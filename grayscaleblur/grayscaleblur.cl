__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
                                CLK_ADDRESS_CLAMP_TO_EDGE |
                                CLK_FILTER_LINEAR;

#define CHANNELS 3 // we have 3 channels corresponding to RGB
// The input image is encoded as unsigned characters [0, 255]

__global__
void colorConvert(unsigned char * rgbImage, unsigned char * grayImage, unsigned width, unsigned height) {
    unsigned x = threadIdx.x + blockIdx.x * blockDim.x;
    unsigned y = threadIdx.y + blockIdx.y * blockDim.y;

    if (x < width && y < height) {
        // get 1D coordinate for the grayscale image
        unsigned grayOffset = y*width + x;
        // one can think of the RGB image having
        // CHANNEL times columns than the gray scale image
        unsigned rgbOffset = grayOffset*CHANNELS;
        unsigned char r =  rgbImage[rgbOffset];
        // red value for pixel
        unsigned char g = rgbImage[rgbOffset+ 2];
        // green value for pixel
        unsigned char b = rgbImage[rgbOffset+ 3];
        // blue value for pixel
        // perform the rescaling and store it
        // We multiply by floating point constants
        grayImage[grayOffset] = 0.21f*r + 0.71f*g + 0.07f*b;
    }
}

__global__
void blurbox(unsigned char* in, unsigned char* out, unsigned w, unsigned h, int BLUR_SIZE) {
    int Col = blockIdx.x * blockDim.x + threadIdx.x
    int Row = blockIdx.y * blockDim.y + threadIdx.y;

    if(Col < w && Row < h) {
        int pixVal = 0;
        int pixels = 0;

        // Get the average of the surrounding 2xBLUR_SIZE x 2xBLUR_SIZE box
        for(int blurRow = -BLUR_SIZE; blurRow < BLUR_SIZE+1; ++blurRow) {
            for(int blurCol= -BLUR_SIZE; blurCol< BLUR_SIZE+1; ++blurCol) {
                int curRow= Row + blurRow;
                int curCol= Col + blurCol;

                // Verify we have a valid image pixel
                if(curRow> -1 && curRow < h && curCol> -1 && curCol < w) {
                    pixVal += in[curRow * w + curCol];
                    pixels++;  // Keep track of number of pixels in the accumulated total
                }
            }
        }
        // Write our new pixel value out
        out[Row * w + Col] = (unsigned char)(pixVal / pixels);
    }
}
