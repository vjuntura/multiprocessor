#include<stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){
   /* 2D array declaration*/
   int mat1[100][100];
   int mat2[100][100];
   int sum[100][100];

   //init srand
   srand(time(NULL));

   //init mat arrays with random int
   int i, j;
   for(i=0; i<100; i++) {
      for(j=0;j<100;j++) {
         mat1[i][j] = i+j;//rand() % 50;
         mat2[i][j] = i+j;//rand() % 50;
      }
   }

   //Measure execution time
   clock_t begin = clock();

   //sum mat1 and mat2
   for(i=0; i<100; i++) {
      for(j=0;j<100;j++) {
          sum[i][j] = mat1[i][j] + mat2[i][j];
      }
   }

   clock_t end = clock();
   double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

   //Print result sum
   int totalsum = 0;
   for(i=0; i<100; i++) {
      for(j=0;j<100;j++) {
          totalsum += sum[i][j];
      }
    }
    printf("final result: %d\n", totalsum);
    printf("Excecution time %f\n", time_spent);

    return 0;
}
