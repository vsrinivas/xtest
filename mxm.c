#include <stdint.h>

static uint32_t* V(uint32_t *A, int i, int j, int cols) {
	return &A[(i * cols) + j];
}

void multiply(uint32_t *C, const uint32_t *A, const uint32_t *B,
              int a_rows, int a_cols, int b_cols)
{
	int i, j, k;

	for (i = 0; i < a_rows; i++) {
		for (k = 0; k < a_cols; k++) {
			for (j = 0; j < b_cols; j++) {
				*V(C, i, j, b_cols) += *V(A, i, k, a_cols) *
						       *V(B, k, j, b_cols);
			}
		}
	}
}






int main(int argc, char *argv[]) {
	uint32_t A[4][4] = {};
	uint32_t B[4][4] = {};
	uint32_t C[4][4] = {};

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			A[i][j] = 1;
			B[i][j] = 1;
		}
	B[0][0] = 1;
	B[1][1] = 1;
	B[2][2] = 1;
	B[3][3] = 1;

	multiply(C, A, B, 4, 4, 4);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			printf("%d ", C[i][j]);
		printf("\n");
	}

	printf("\n");


	uint32_t W[4][1] = {};
	uint32_t X[1][4] = {};
	uint32_t Y[4][4] = {};
	X[0][0]=X[0][1]=X[0][2]=X[0][3]=1;
	W[0][0]=W[1][0]=W[2][0]=W[3][0]=1;
	multiply(Y, W, X, 4, 1, 4);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			printf("%d ", Y[i][j]);
		printf("\n");
	}
}
