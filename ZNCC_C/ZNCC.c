#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>


#define windowX 5
#define windowY 5

void zncc(unsigned char* IL, unsigned char* IR,
          unsigned width, unsigned height,
          int Max_Disp, int Min_Disp,
          unsigned char* DisparityMap){}

int main(int argc, char *argv[]) {

    // Read images 
    const char* filenameL = "imageL.png";
    const char* filenameR = "imageR.png";
    const char* out_filename = "testi.png";

    unsigned char* IL = 0;
    unsigned char* IR = 0;
    
    unsigned width1, height1, width2, height2;
    unsigned bitdepth = 8;

    //Decode
    unsigned error = lodepng_decode_file(&IL, &width1, &height1, filenameL, LCT_GREY, bitdepth);
    unsigned error = lodepng_decode_file(&IR, &width2, &height2, filenameR, LCT_GREY, bitdepth);
    unsigned size1 = width1*height1;
    unsigned size2 = width2*height2;

    if(size1 != size2){
        printf("Images sizes unvalid");
        return -1;
    }

    int Max_Disp = 50;
    int Min_Disp = 0;
    int Win_Size = 5;
    int Number_of_win_pixels=Win_Size^2;

    // Disparity maps
    unsigned char* DisparityMapL2R;
    unsigned char* DisparityMapR2L;

    DisparityMapL2R = (char*)malloc(size1*sizeof(unsigned char));
    DisparityMapR2L = (char*)malloc(size2*sizeof(unsigned char));

    // Calculate disparity maps with zncc
    
    // Left to right
    zncc(IL, IR, width1, height1, Max_Disp, Max_Disp, DisparityMapL2R);
    // Right to left
    zncc(IR, IL, width1, height1, Max_Disp, Max_Disp, DisparityMapR2L);

}

void zncc(unsigned char* IL, unsigned char* IR,
          unsigned width, unsigned height,
          int max_disp, int min_disp,
          unsigned char* DisparityMap)
{
    // Disparity map computation
    int size = width*height; // Size of the image
    int window_size = windowX*windowY; // Block size

    int i, j; // Indices for rows and colums respectively
    int i_b, j_b; // Indices within the block
    int ind_l, ind_r; // Indices of block values within the whole image
    int d; // Disparity value
    float cl, cr; // centered values of a pixel in the left and right images;
    
    float lbmean, rbmean; // Blocks means for left and right images
    float lbstd, rbstd; // Left block std, Right block std
    float current_score; // Current ZNCC value
    
    int best_d;
    float best_score;
    
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            // Searching for the best d for the current pixel
            best_d = max_disp;
            best_score = -1;
            for (d = min_disp; d <= max_disp; d++) {
                // Calculating the blocks' means
                lbmean = 0;
                rbmean = 0;
                for (i_b = -windowY/2; i_b < windowY/2; i_b++) {
                    for (j_b = -windowX/2; j_b < windowX/2; j_b++) {
                        // Borders checking
                        if (!(i+i_b >= 0) || !(i+i_b < height) || !(j+j_b >= 0) || !(j+j_b < width) || !(j+j_b-d  >= 0) || !(j+j_b-d < width)) {
                                continue;
                        }
                        // Calculatiing indices of the block within the whole image
                        ind_l = (i+i_b)*width + (j+j_b);
                        ind_r = (i+i_b)*width + (j+j_b-d);
                        // Updating the blocks' means
                        lbmean += IL[ind_l];
                        rbmean += IR[ind_r];
                    }
                }
                lbmean /= window_size;
                rbmean /= window_size;
                
                // Calculating ZNCC for given value of d
                lbstd = 0;
                rbstd = 0;
                current_score = 0;
                
                // Calculating the nomentaor and the standard deviations for the denominator
                for (i_b = -windowY/2; i_b < windowY/2; i_b++) {
                    for (j_b = -windowX/2; j_b < windowX/2; j_b++) {
                        // Borders checking
                        if (!(i+i_b >= 0) || !(i+i_b < height) || !(j+j_b >= 0) || !(j+j_b < width) || !(j+j_b-d  >= 0) || !(j+j_b-d < width)) {
                                continue;
                        }
                        // Calculatiing indices of the block within the whole image
                        ind_l = (i+i_b)*width + (j+j_b);
                        ind_r = (i+i_b)*width + (j+j_b-d);
                            
                        cl = IL[ind_l] - lbmean;
                        cr = IR[ind_r] - rbmean;
                        lbstd += cl*cl;
                        rbstd += cr*cr;
                        current_score += cl*cr;
                    }
                }
                // Normalizing the denominator
                current_score /= sqrt(lbstd)*sqrt(rbstd);
                // Selecting the best disparity
                if (current_score > best_score) {
                    best_score = current_score;
                    best_d = d;
                }
            }
            DisparityMap[i*width+j] = (int) abs(best_d); // Considering both Left to Right and Right to left disparities
        } 
    }       
}