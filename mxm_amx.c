#include <stdint.h>
#include <stdlib.h>
#include <immintrin.h>

__attribute__((pure))
static uint8_t* V(uint8_t *A, int i, int j, int cols) {
	return &A[(i * cols) + j];
}


static 
void multiply(uint8_t *C, const uint8_t *A, const uint8_t *B,
              int a_rows, int a_cols, int b_cols)
{
        int i, j, k;
	#define blocksize (16)
	int jb, kb;

	for (int ii = 0; ii < a_rows; ii += blocksize) {
		for (int jj = 0; jj < b_cols; jj += blocksize) {
			for (int kk = 0; kk < a_cols; kk += blocksize) {
				uint8_t BB[blocksize][blocksize];
				jb=0;
				for (j = jj; j < jj + blocksize; j++) {
					kb=0;
					for (k = kk; k < kk + blocksize; k++) {
						BB[jb][kb] = *V(B, k, j, b_cols);
						kb++;
					}
					jb++;
				}


				for (i = ii; i < ii + blocksize; i++) {
					for (j = jj, jb = 0; j < jj + blocksize; j++, jb++) {
						uint8_t t = *V(C, i, j, b_cols);
						for (k = kk, kb = 0; k < kk + blocksize; k++, kb++) {
							t += *V(A, i, k, a_cols) * BB[jb][kb];
						}
						*V(C, i, j, b_cols) = t;
					}
				}
			}
		}
	}
}


#define N (4992)
uint8_t A[N][N] = {};
uint8_t B[N][N] = {};
uint8_t C[N][N] = {};
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
