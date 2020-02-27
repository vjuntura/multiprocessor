#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define windowX 4
#define windowY 4

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
    const char* dimage1out_filename = "dimage1.png";
    const char* dimage2out_filename = "dimage2.png";

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

    int max_disp = 255;
    int min_disp = 0;

    // Disparity maps
    unsigned char* DisparityMapL2R;
    unsigned char* DisparityMapR2L;
    unsigned char* result;

    DisparityMapL2R = (unsigned char*)malloc(size1*sizeof(unsigned char));
    DisparityMapR2L = (unsigned char*)malloc(size2*sizeof(unsigned char));
    result = (unsigned char*)malloc(size1*sizeof(unsigned char));

    // Calculate disparity maps with zncc

    // Left to right
    zncc(IL, IR, width1, height1, max_disp, min_disp, DisparityMapL2R);
    /*printf("Disparity left\n");
    for(int i=0; i < width1; i++){
        for(int j=0; j < height1; j++){
            printf("%d\n", DisparityMapL2R[i*height1 + j]);
        }
    }*/

    lodepng_encode_file(dimage1out_filename, DisparityMapL2R, width1, height1, LCT_GREY, bitdepth);
    // Right to left
    zncc(IR, IL, width1, height1, max_disp, min_disp, DisparityMapR2L);
    /*printf("Disparity right\n");
    for(int i=0; i < width2; i++){
        for(int j=0; j < height2; j++){
            printf("%d\n", DisparityMapR2L[i*height2 + j]);
        }
    }*/
    lodepng_encode_file(dimage2out_filename, DisparityMapR2L, width1, height1, LCT_GREY, bitdepth);

    post_processing(DisparityMapL2R, DisparityMapR2L, width1, height1, max_disp, size1, result);

    lodepng_encode_file(out_filename, result, width1, height1, LCT_GREY, bitdepth);

    free(DisparityMapL2R);
    free(DisparityMapR2L);
    free(result);
    free(IR);
    free(IL);
}

void zncc(unsigned char* IL, unsigned char* IR,
          unsigned width, unsigned height,
          int max_disp, int min_disp,
          unsigned char* DisparityMap)
{

    int sum_of_Left_win = 0;
    int sum_of_Right_win = 0;
    int Mean_win_L = 0;
    int Mean_win_R = 0;
    int Number_of_win_pixels = windowX * windowY;
    int NominatorL2R = 0;
    int Denominator1L2R = 0;
    int Denominator2L2R = 0;
    int ZNCC_VALUE_Left_to_Right = 0;
    int CurrentMaximumL2R = -1;
    int BestDisparityValueL2R = 0;

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            for (int d = 0; d < max_disp; d++) {
                for (int Win_Y = 0; windowY < Win_Y; Win_Y++) {
                    for (int Win_X = 0; Win_X < windowX; Win_X++) {
                        //Calculate mean
                        sum_of_Left_win = sum_of_Left_win + IL[i + Win_X, j + Win_Y + d];
                        sum_of_Right_win = sum_of_Right_win + IR[i + Win_X, j + Win_Y];
                    }
                }

                Mean_win_L = sum_of_Left_win / Number_of_win_pixels;
                Mean_win_R = sum_of_Right_win / Number_of_win_pixels;

                for (int Win_Y = 0; windowY < Win_Y; Win_Y++) {
                    for (int Win_X = 0; Win_X < windowX; Win_X++) {
                        //Calculate zncc
                        NominatorL2R = NominatorL2R + (IL[i+Win_X,j+Win_Y+d]-Mean_win_L)*(IR[i+Win_X,j+Win_Y]-Mean_win_R);
                        Denominator1L2R = Denominator1L2R + (IL[i+Win_X,j+Win_Y+d]-Mean_win_L)*(IL[i+Win_X,j+Win_Y+d]-Mean_win_L);
                        Denominator2L2R = Denominator2L2R + (IR[i+Win_X,j+Win_Y]-Mean_win_R)*(IR[i+Win_X,j+Win_Y]-Mean_win_R);
                    }
                }

                ZNCC_VALUE_Left_to_Right = NominatorL2R/(sqrt(Denominator1L2R*Denominator2L2R));

                if (ZNCC_VALUE_Left_to_Right > CurrentMaximumL2R) {
                    CurrentMaximumL2R=ZNCC_VALUE_Left_to_Right;
                    BestDisparityValueL2R=d;
                }
            }
            DisparityMap[i * width + j]=(unsigned char)abs(BestDisparityValueL2R);
        }
    }




    // Calculate disparity map
/*    int window_size = windowX*windowY;

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

    printf("Doing calculations \n");
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
    }*/
}

void post_processing(unsigned char* IL, unsigned char* IR,
                      unsigned width, unsigned height,
                      int Max_Disp, unsigned size,
                      unsigned char* result) {

    int threshold = 12;
    //unsigned char* tempMap = (unsigned char*)malloc(size*sizeof(unsigned char));

    int color_nearest = 0;

    //TODO ei ehkä toimi, katso matlabista j-window
    //Simplest form of neigherest neighbour
    printf("Post processing...\n");
/*    for (int i = 0; i < size; i++) {
        if (abs(IL[i] - IR[i]) < threshold) {
            result[i] = IL[i];
            color_nearest = result[i];
        } else {
            result[i] = color_nearest;
        }
    }*/

    for(int i=0; i < width; i++){
        for(int j=0; j < height; j++){
            if (abs(IL[i*height + j] - IR[i*height + j]) < threshold) {
                result[i*height + j] = IL[i*height + j];
                color_nearest = result[i*height + j];
            } else {
                result[i*height + j] = color_nearest;
            }
        }
    }

    //Blur image for better representation only

}
