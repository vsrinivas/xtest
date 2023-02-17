#include <stdint.h>
#include <stdlib.h>
#include <immintrin.h>

__attribute__((pure))
static uint32_t* V(uint32_t *A, int i, int j, int cols) {
	return &A[(i * cols) + j];
}


static 
void multiply(uint32_t *C, const uint32_t *A, const uint32_t *B,
              int a_rows, int a_cols, int b_cols)
{
        int i, j, k;
	#define blocksize (64)
	int jb, kb;

#pragma omp parallel for
	for (int ii = 0; ii < a_rows; ii += blocksize) {
		for (int jj = 0; jj < b_cols; jj += blocksize) {
			for (int kk = 0; kk < a_cols; kk += blocksize) {
				uint32_t BB[blocksize][blocksize] = {};
				jb=0;
				for (j = jj; j < jj + blocksize; j++, jb++) {
					for (k = kk, kb = 0; k < kk + blocksize; k++, kb++) {
						BB[jb][kb] = *V(B, k, j, b_cols);
					}
				}


				for (i = ii; i < ii + blocksize; i += 2) {
					for (j = jj, jb = 0; j < jj + blocksize; j += 2, jb +=2) {
					uint32_t t00, t01, t10, t11;
					t00 = *V(C, i, j, b_cols);
					t01 = *V(C, i, j+1, b_cols);
					t10 = *V(C, i+1, j, b_cols);
					t11 = *V(C, i+1, j+1, b_cols);

					for (k = kk, kb = 0; k < kk + blocksize; k++, kb++) {
						t00 += *V(A, i, k, a_cols) * BB[jb][kb];
						t01 += *V(A, i, k, a_cols) * BB[jb+1][kb];
						t10 += *V(A, i+1, k, a_cols) * BB[jb][kb];
						t11 += *V(A, i+1, k, a_cols) * BB[jb+1][kb];
					}

					*V(C, i, j, b_cols) = t00;
					*V(C, i, j + 1, b_cols) = t01;
					*V(C, i+1, j, b_cols) = t10;
					*V(C, i+1, j+1, b_cols) = t11;
					}
				}
			}
		}
	}
}


#define N (4992)
uint32_t A[N][N] = {};
uint32_t B[N][N] = {};
uint32_t C[N][N] = {};
int main(int argc, char *argv[]) {

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++) {
			A[i][j] = 1;
			if (i==j)
			B[i][j] = 1;
		}

	multiply(C, A, B, N, N, N);

	printf("%d\n", C[1][1]);
}
