#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define windowX 5
#define windowY 5

void zncc(unsigned char* IL, unsigned char* IR,
          unsigned width, unsigned height,
          int Max_Disp, int Min_Disp,
          unsigned char* DisparityMap);


void post_processing(unsigned char* IL, unsigned char* IR,
                      unsigned width, unsigned height,
                      int Max_Disp, unsigned size,
                      unsigned char* result);


int main(int argc, char *argv[]) {

    // Read images
    const char* filenameL = "im0.png";
    const char* filenameR = "im1.png";

    const char* out_filename = "testi.png";

    unsigned char* IL = 0;
    unsigned char* IR = 0;

    unsigned width1, height1, width2, height2;
    unsigned bitdepth = 8;

    //Decode
    lodepng_decode_file(&IL, &width1, &height1, filenameL, LCT_GREY, bitdepth);
    lodepng_decode_file(&IR, &width2, &height2, filenameR, LCT_GREY, bitdepth);
    unsigned size1 = width1*height1;
    unsigned size2 = width2*height2;

    if(size1 != size2){
        printf("Images sizes unvalid");
        return -1;
    }

    int max_disp = 50;
    int min_disp = 0;

    // Disparity maps
    unsigned char* DisparityMapL2R;
    unsigned char* DisparityMapR2L;

    DisparityMapL2R = (unsigned char*)malloc(size1*sizeof(unsigned char));
    DisparityMapR2L = (unsigned char*)malloc(size2*sizeof(unsigned char));

    // Calculate disparity maps with zncc

    // Left to right
    zncc(IL, IR, width1, height1, max_disp, min_disp, DisparityMapL2R);
    // Right to left
    zncc(IR, IL, width1, height1, max_disp, min_disp, DisparityMapR2L);

}

void zncc(unsigned char* IL, unsigned char* IR,
          unsigned width, unsigned height,
          int max_disp, int min_disp,
          unsigned char* DisparityMap)
{
    // Calculate disparity map
    int window_size = windowX*windowY;

    int i, j;
    int i_b, j_b;
    int ind_l, ind_r;
    int dp_value;

    float centerL, centerR;
    float lwmean, rwmean;
    float lbstd, rbstd;
    float current_disp;

    int best_disp;
    float currentMaximum;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            // Search for the best dp_value
            best_disp = max_disp;
            currentMaximum = -1;
            for (dp_value = min_disp; dp_value <= max_disp; dp_value++) {
                // Window mean
                lwmean = 0;
                rwmean = 0;
                for (i_b = -windowY/2; i_b < windowY/2; i_b++) {
                    for (j_b = -windowX/2; j_b < windowX/2; j_b++) {
                        // Check borders
                        if (!(i+i_b >= 0) || !(i+i_b < height) || !(j+j_b >= 0) || !(j+j_b < width) || !(j+j_b-dp_value  >= 0) || !(j+j_b-dp_value < width)) {
                                continue;
                        }
                        // Calculatiing indices of the block within the whole image // WTF!?
                        ind_l = (i+i_b)*width + (j+j_b);
                        ind_r = (i+i_b)*width + (j+j_b-dp_value);
                        // Updating the blocks' means
                        lwmean += IL[ind_l];
                        rwmean += IR[ind_r];
                    }
                }

                lwmean /= window_size;
                rwmean /= window_size;

                lbstd = 0;
                rbstd = 0;
                current_disp = 0;

                // Calculating the nomentaor and the standard deviations for the denominator
                for (i_b = -windowY/2; i_b < windowY/2; i_b++) {
                    for (j_b = -windowX/2; j_b < windowX/2; j_b++) {
                        // Borders checking
                        if (!(i+i_b >= 0) || !(i+i_b < height) || !(j+j_b >= 0) || !(j+j_b < width) || !(j+j_b-dp_value  >= 0) || !(j+j_b-dp_value < width)) {
                                continue;
                        }
                        // Calculatiing indices of the block within the whole image
                        ind_l = (i+i_b)*width + (j+j_b);
                        ind_r = (i+i_b)*width + (j+j_b-dp_value);

                        centerL = IL[ind_l] - lwmean;
                        centerR = IR[ind_r] - rwmean;
                        lbstd += centerL*centerL;
                        rbstd += centerR*centerR;
                        current_disp += centerL*centerR;
                    }
                }
                // Normalizing the denominator
                current_disp /= sqrt(lbstd)*sqrt(rbstd);
                // Selecting the best disparity
                if (current_disp > currentMaximum) {
                    currentMaximum = current_disp;
                    best_disp = dp_value;
                }
            }
            DisparityMap[i*width+j] = (int) abs(best_disp); // Considering both Left to Right and Right to left disparities
        }
    }
}

void post_processing(unsigned char* IL, unsigned char* IR,
                      unsigned width, unsigned height,
                      int Max_Disp, unsigned size,
                      unsigned char* result) {

    int threshold = 12;
    tempMap = (char*)malloc(size*sizeof(unsigned char));

    int color_nearest = 0;

    //TODO ei ehkä toimi, katso matlabista j-window
    //Simplest form of neigherest neighbour
    for (int i = 0; i < size; i++) {
        if (abs(IL[i] - IR[i])< threshold) {
            tempMap[i] = IL[i];
            color_nearest = tempMap[i];
        } else {
            tempMap[i] = color_nearest;
        }
    }

    //Blur image for better representation only

}
