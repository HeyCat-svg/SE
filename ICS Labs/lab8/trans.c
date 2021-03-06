/* name: DengShiyi stu_no: 518021910184
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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void deal32x32(int M, int N, int A[N][M], int B[M][N]);
void deal64x64(int M, int N, int A[N][M], int B[M][N]);
void deal61x67(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){
    int ii, jj, i, j, v0, v1, v2, v3, v4, v5, v6, v7;
    // deal with 32*32 matrix
    if (N ==32 && M == 32){
        // the block size is 8*8
        for (ii = 0; ii < 32; ii+=8){
            for (jj = 0; jj < 32;jj+=8){
                for (i = ii; i < ii + 8; ++i){
                    v0 = A[i][jj];
                    v1 = A[i][jj + 1];
                    v2 = A[i][jj + 2];
                    v3 = A[i][jj + 3];
                    v4 = A[i][jj + 4];
                    v5 = A[i][jj + 5];
                    v6 = A[i][jj + 6];
                    v7 = A[i][jj + 7];
                    B[jj][i] = v0;
                    B[jj + 1][i] = v1;
                    B[jj + 2][i] = v2;
                    B[jj + 3][i] = v3;
                    B[jj + 4][i] = v4;
                    B[jj + 5][i] = v5;
                    B[jj + 6][i] = v6;
                    B[jj + 7][i] = v7;
                }
            }
        }
    }
    // deal with 64*64 matrix
    else if (N == 64 && M == 64){
        // the block size is 8*8
        for (ii = 0; ii < 64; ii += 8){
            for (jj = 0; jj < 64; jj += 8){
                for (i = ii; i < ii + 4; ++i){
                    v0 = A[i][jj];
                    v1 = A[i][jj + 1];
                    v2 = A[i][jj + 2];
                    v3 = A[i][jj + 3];
                    v4 = A[i][jj + 4];
                    v5 = A[i][jj + 5];
                    v6 = A[i][jj + 6];
                    v7 = A[i][jj + 7];
                    B[jj][i] = v0;
                    B[jj + 1][i] = v1;
                    B[jj + 2][i] = v2;
                    B[jj + 3][i] = v3;
                    B[jj][i + 4] = v4;
                    B[jj + 1][i + 4] = v5;
                    B[jj + 2][i + 4] = v6;
                    B[jj + 3][i + 4] = v7;
                }
                for (i = jj; i < jj + 4; ++i){
                    v0 = A[ii + 4][i];
                    v1 = A[ii + 5][i];
                    v2 = A[ii + 6][i];
                    v3 = A[ii + 7][i];
                    v4 = B[i][ii + 4];
                    v5 = B[i][ii + 5];
                    v6 = B[i][ii + 6];
                    v7 = B[i][ii + 7];
                    B[i][ii + 4] = v0;
                    B[i][ii + 5] = v1;
                    B[i][ii + 6] = v2;
                    B[i][ii + 7] = v3;
                    B[i + 4][ii] = v4;
                    B[i + 4][ii + 1] = v5;
                    B[i + 4][ii + 2] = v6;
                    B[i + 4][ii + 3] = v7;
                }
                for (i = ii + 4; i < ii + 8; ++i){
                    v0 = A[i][jj + 4];
                    v1 = A[i][jj + 5];
                    v2 = A[i][jj + 6];
                    v3 = A[i][jj + 7];
                    B[jj + 4][i] = v0;
                    B[jj + 5][i] = v1;
                    B[jj + 6][i] = v2;
                    B[jj + 7][i] = v3;
                }
            }
        }
    }
    // deal with 61*67 matrix
    else{
        // the block size is 16*16
        for (ii = 0; ii < N; ii += 16){
            for (jj = 0; jj < M; jj += 16){
                for (i = ii; i < ii + 16 && i < N; ++i){
                    for (j = jj; j < jj + 16 && j < M; ++j){
                        B[j][i] = A[i][j];
                    }
                }
            }
        }
    }
}

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

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

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

