/* id:haoyang
 *
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */

void check(int i, int* t,int* t2){
  if(i == 0){
    *t = 0;
    *t2 = 0;
  }
  if(i == 2){
    *t = 4;
    *t2 = 4;
  }
  if(i == 3){
    *t = 0;
    *t2 = 4;
  }
  if(i == 1){
    *t = 4;
    *t2 = 0;
  }
}


char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{

    REQUIRES(M > 0);
    REQUIRES(N > 0);
int col,row,temp;


 if(M == 32){//splits into 8 x 8
      for(row = 0; row < N; row +=8){
        for(col = 0;col < M; col +=8){
          for(int i = row; i< row+8; i++){// for each 8 we go thru
            for(int j = col; j < col +8; j++){
              if(i == j){
                temp =  A[j][j];
              }
              if(i != j){
                B[j][i] = A[i][j];//non diags
              }
            }

            if(row == col){ //diagonals
              temp = A[i][i];
              B[i][i] = temp;//transpose indexes
            }
          }


          }
      }}

    if(M == 64){
      for(row = 0; row < N; row +=4){//split into 4x4
        for(col = 0;col < M; col +=4){
          for(int i = row; i< (((row+4) <= N) ? (row+4) : N); i++){//used from
            for(int j = col; j < (((col+4) <= M) ? (col+4) : M); j++){//61x67
              int d = A[i][j];
              B[j][i] = d;//transpose indexes
            }


          }


          }
        }
}

 if(M == 61){
      for(row = 0; row < N; row +=22){
        for(col = 0;col < M; col +=22){//we only need check the smaller of two
          for(int i = row; i< (((row+22) <= N) ? (row+22) : N); i++){
            for(int j = col; j < (((col+22) <= M) ? (col+22) : M); j++){
              int d = A[i][j];
              B[j][i] = d;//tranpose indexes
            }


          }


          }
        }
}


 ENSURES(is_transpose(M, N, A, B));
};

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

