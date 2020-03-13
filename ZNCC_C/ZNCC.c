//2d array indexing: https://stackoverflow.com/questions/2151084/map-a-2d-array-onto-a-1d-array

//Compilation: gcc ZNCC.c lodepng.c -Wall -o paska -lm

#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>


#define windowX 20
#define windowY 14

void zncc(unsigned char* IL, unsigned char* IR,
          unsigned width, unsigned height,
          int Max_Disp, int Min_Disp,
          unsigned char* DisparityMap);

void post_processing(unsigned char* IL, unsigned char* IR,
                      unsigned width, unsigned height,
                      int Max_Disp, unsigned size,
                      unsigned char* resultMap);

void convertgray(const uint8_t* IL, const uint8_t* IR,
                uint8_t* grayL, uint8_t* grayR, uint32_t w, uint32_t h);


int main(int argc, char *argv[]) {

    // Read images
    const char* filenameL = "im0.png";
    const char* filenameR = "im1.png";

    const char* out_filename = "output.png";
    const char* dimage1out_filename = "disparityMapL.png";
    const char* dimage2out_filename = "disparityMapR.png";

    uint8_t* original_IL = 0;
    uint8_t* original_IR = 0;

    uint8_t* gray_IL = 0;
    uint8_t* gray_IR = 0;


    uint32_t width1, height1, width2, height2;
    unsigned bitdepth = 8;

    //Decode
    lodepng_decode32_file(&original_IL, &width1, &height1, filenameL);//, LCT_GREY, bitdepth);
    lodepng_decode32_file(&original_IR, &width2, &height2, filenameR);//, LCT_GREY, bitdepth);
    unsigned size1 = width1*height1;
    unsigned size2 = width2*height2;

    if(size1 != size2){
        printf("Images sizes unvalid");
        return -1;
    }

    int max_disp = 60;
    int min_disp = 0;

    //New sizes after rescaling
    uint32_t size = (width1 / 4) * (height1 / 4);
    uint32_t width = width1 / 4;
    uint32_t height = height1 / 4;

    gray_IL = (uint8_t*)malloc(size * sizeof(uint8_t));
    gray_IR = (uint8_t*)malloc(size * sizeof(uint8_t));

    //Convert to grayscale
    convertgray(original_IL, original_IR, gray_IL, gray_IR, width1, height1);

    // Disparity maps
    uint8_t* DisparityMapL2R;
    uint8_t* DisparityMapR2L;
    uint8_t* resultMap;

    DisparityMapL2R = (uint8_t*)malloc(size * sizeof(uint8_t));
    DisparityMapR2L = (uint8_t*)malloc(size * sizeof(uint8_t));
    resultMap = (uint8_t*)malloc(size * sizeof(uint8_t));

    // Calculate disparity maps with zncc
    // Left to right
    zncc(gray_IL, gray_IR, width, height, max_disp, min_disp, DisparityMapL2R);
    lodepng_encode_file(dimage1out_filename, DisparityMapL2R, width, height, LCT_GREY, bitdepth);

    // Right to left
    zncc(gray_IR, gray_IL, width, height, min_disp, -max_disp, DisparityMapR2L);
    lodepng_encode_file(dimage2out_filename, DisparityMapR2L, width, height, LCT_GREY, bitdepth);

    post_processing(DisparityMapL2R, DisparityMapR2L, width, height, max_disp, size, resultMap);
    lodepng_encode_file(out_filename, resultMap, width, height, LCT_GREY, bitdepth);

    free(DisparityMapL2R);
    free(DisparityMapR2L);
    free(resultMap);
    free(original_IL);
    free(original_IR);
    free(gray_IL);
    free(gray_IR);
}

void zncc(uint8_t* IL, uint8_t* IR,
          uint32_t width, uint32_t height,
          int max_disp, int min_disp,
          uint8_t* DisparityMap)
{

    float sum_of_Left_win = 0;
    float sum_of_Right_win = 0;
    int Number_of_win_pixels = windowX * windowY;
    float Nominator = 0;
    float Denominator1 = 0;
    float Denominator2 = 0;
    float CurrentMaximum = -1;
    int BestDisparityValue = 0;


    printf("Calculating...\n");
    printf("w %d, h %d\n", width, height);


    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            CurrentMaximum = -1;
            BestDisparityValue = max_disp;
            for (int d = min_disp; d <= max_disp; d++) {
                sum_of_Left_win = 0;
                sum_of_Right_win = 0;


                for (int Win_Y = -windowY/2; Win_Y < windowY/2; Win_Y++) {
                    for (int Win_X = -windowX/2; Win_X < windowX/2; Win_X++) {

                        // Exclude borders
                        if (!(i + Win_X >= 0) || !(i + Win_X < height) ||
                            !(j + Win_Y >= 0) || !(j + Win_Y < width) ||
                            !(j + Win_Y - d  >= 0) || !(j + Win_Y - d < width)) {
                            continue;
                        }

                        // [width * row + col]
                        int row = width * (i + Win_Y);
                        int col = j + Win_X;

                        //Calculate mean
                        sum_of_Left_win += IL[row + col];
                        sum_of_Right_win += IR[row + (col - d)];
                    }
                }


                sum_of_Left_win = sum_of_Left_win / Number_of_win_pixels;
                sum_of_Right_win = sum_of_Right_win / Number_of_win_pixels;

                Nominator = 0;
                Denominator1 = 0;
                Denominator2 = 0;

                for (int Win_Y = -windowY/2; Win_Y < windowY/2; Win_Y++) {
                    for (int Win_X = -windowX/2; Win_X < windowX/2; Win_X++) {

                        // Exclude borders
                        if (!(i + Win_X >= 0) || !(i + Win_X < height) ||
                            !(j + Win_Y >= 0) || !(j + Win_Y < width) ||
                            !(j + Win_Y - d  >= 0) || !(j + Win_Y - d < width)) {
                            continue;
                        }

                        //Calculate zncc
                        int row = width * (i + Win_Y);
                        int col = j + Win_X;

                        int centerL = IL[row + col] - sum_of_Left_win;
                        int centerR = IR[row + (col -d)] - sum_of_Right_win;

                        Denominator1 += centerL*centerL;
                        Denominator2 += centerR*centerR;
                        Nominator += centerL*centerR;
                    }
                }


                Nominator /= sqrt(Denominator1) * sqrt(Denominator2);

                if (Nominator > CurrentMaximum) {
                    CurrentMaximum = Nominator;
                    BestDisparityValue = d;
                }

            }
            DisparityMap[width * i + j] = (uint8_t) abs(BestDisparityValue);
        }
    }
}

void post_processing(uint8_t* IL, uint8_t* IR,
                      uint32_t width, uint32_t height,
                      int Max_Disp, uint32_t size,
                      uint8_t* resultMap) {

    int threshold = 12;
    int color_nearest = 0;
    uint8_t max = 0;
    uint8_t min = 255;

    //Simplest form of neigherest neighbour
    printf("Post processing...\n");
    for(int i = 0; i < height; i++){
        color_nearest = 0;
        for(int j = 0; j < width; j++){
            if (abs(IL[i*width + j] - IR[i*width + j]) < threshold) {
                resultMap[i*width + j] = IL[i*width + j];
                color_nearest = resultMap[i*width + j];
            } else {
                resultMap[i*width + j] = color_nearest;
            }
        }
    }

    //Perform normalization
    for (uint32_t i = 0; i < size; i++) {
        if (resultMap[i] > max)
            max = resultMap[i];
        if (resultMap[i] < min)
            min = resultMap[i];
    }

    for (uint32_t i = 0; i < size; i++) {
        resultMap[i] = (uint8_t) (255 * (resultMap[i] - min) / (max - min));
    }

}

//Convert to grayscale
void convertgray(const uint8_t* IL, const uint8_t* IR,
                uint8_t* grayL, uint8_t* grayR, uint32_t w, uint32_t h) {

	int32_t i, j, original_i, original_j;
    //W and h of the grayscale image
    int32_t new_width = w / 4, new_height= h / 4;

	for (i = 0; i < new_height; i++) {
	    for (j = 0; j < new_width; j++) {
	        // Calculating indices of the original image
	        original_i = (4*i-1*(i > 0));
	        original_j = (4*j-1*(j > 0));

            // Convert to grayscale
            grayL[i * new_width + j] = 0.2126 * IL[original_i*(4*w) + 4 * original_j]
                                + 0.7152 * IL[original_i * (4 * w) + 4 * original_j + 1]
                                + 0.0722 * IL[original_i * (4 * w) + 4 * original_j + 2];

            grayR[i * new_width + j] = 0.2126 * IR[original_i * (4 * w) + 4 * original_j]
                                    + 0.7152 * IR[original_i * (4 * w) + 4 * original_j + 1]
                                    + 0.0722 * IR[original_i * (4 * w) + 4 * original_j + 2];
		}
	}
}
