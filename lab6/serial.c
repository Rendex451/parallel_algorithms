#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VISIBLE_SIZE 10

void init_matrix(int *matrix, int rows, int cols);
void print_matrix(int *matrix, int rows, int cols, char *name);

int main(int argc, char *argv[]) {
    int N;
    if (argc == 2) {
        N = atoi(argv[1]);
    } else {
        printf("Usage: %s <matrix_size>\n", argv[0]);
        return 1;
    }

    if (N <= 0) {
        printf("Error: Matrix size must be positive.\n");
        return 1;
    }

    int *A = (int*)malloc(N * N * sizeof(int));
    int *B = (int*)malloc(N * N * sizeof(int));
    int *C = (int*)malloc(N * N * sizeof(int));

    if (!A || !B || !C) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    srand(time(NULL));
    init_matrix(A, N, N);
    init_matrix(B, N, N);

    printf("Sequential matrix multiplication of %dx%d matrices\n", N, N);

    if (N <= MAX_VISIBLE_SIZE) {
        print_matrix(A, N, N, "A");
        print_matrix(B, N, N, "B");
    }

    clock_t start = clock();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    if (N <= MAX_VISIBLE_SIZE) {
        print_matrix(C, N, N, "C (result)");
    }

    printf("Time elapsed: %.6f seconds\n", time_spent);

    free(A);
    free(B);
    free(C);

    return 0;
}

void init_matrix(int *matrix, int rows, int cols) {
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (rand() % 21) - 10; 
    }
}

void print_matrix(int *matrix, int rows, int cols, char *name) {
    printf("Matrix %s:\n", name);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%4d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}