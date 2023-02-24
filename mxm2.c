#include <stdint.h>
#include <stdlib.h>
#include <immintrin.h>

__attribute__((pure))
static inline uint32_t* V(uint32_t *A, int i, int j, int cols) {
	return &A[(i * cols) + j];
}


static 
void multiply(uint32_t *C, const uint32_t *A, const uint32_t *B,
              int a_rows, int a_cols, int b_cols)
{
        int i, j, k;

#pragma omp parallel for schedule(dynamic)
	for (i = 0; i < a_rows; i++) {
		const uint32_t *KK = V(A, i, 0, a_cols);
                for (k = 0; k < a_cols; k++) {
			const uint32_t K = KK[k];
#pragma omp simd
                	for (j = 0; j < b_cols; j++) {
                                *V(C, i, j, b_cols) += K *
                                                       *V(B, k, j, b_cols);
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
