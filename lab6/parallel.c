#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define MAX_VISIBLE_SIZE 10

void init_matrix(int *matrix, int rows, int cols);
void print_matrix(int *matrix, int rows, int cols, char *name);
void panic(char *message, MPI_Comm comm);

int main(int argc, char **argv) 
{
    int rank, size, N;
    int *A = NULL, *B = NULL, *C = NULL;
    int *A_local, *B_local, *C_local, *B_temp;
    
    MPI_Comm cart_comm;
    int dims[1];
    int periods[1];
    int source_rank, dest_rank;

    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    dims[0] = world_size; 
    periods[0] = 1; 

    MPI_Cart_create(MPI_COMM_WORLD, 1, dims, periods, 1, &cart_comm);

    MPI_Comm_rank(cart_comm, &rank);
    MPI_Comm_size(cart_comm, &size);

    if (argc != 2) {
        if (rank == 0) printf("Usage: mpirun -np <num_processes> ./parallel <matrix_size>\n");
        MPI_Finalize();
        return 1;
    }

    N = atoi(argv[1]);
    if (N <= 0) {
        panic("Matrix size must be positive.\n", cart_comm);
    }
    if (N % size != 0) {
        panic("Matrix size must be divisible by number of processes.\n", cart_comm);
    }
    
    int rows_per_proc = N / size;

    if (rank == 0) {
        A = (int*)malloc(N * N * sizeof(int));
        B = (int*)malloc(N * N * sizeof(int));
        C = (int*)malloc(N * N * sizeof(int));

        srand(time(NULL));
        init_matrix(A, N, N);
        init_matrix(B, N, N);

        printf("Matrix multiplication of %dx%d matrices using %d processes\n", N, N, size);
        if (N <= MAX_VISIBLE_SIZE) {
            print_matrix(A, N, N, "A");
            print_matrix(B, N, N, "B");
        }
    }

    A_local = (int*)malloc(rows_per_proc * N * sizeof(int));
    B_local = (int*)malloc(rows_per_proc * N * sizeof(int));
    B_temp = (int*)malloc(rows_per_proc * N * sizeof(int));
    C_local = (int*)calloc(rows_per_proc * N, sizeof(int));

    double start_time = MPI_Wtime();

    MPI_Scatter(A, rows_per_proc * N, MPI_INT, A_local, rows_per_proc * N, MPI_INT, 0, cart_comm);
    MPI_Scatter(B, rows_per_proc * N, MPI_INT, B_local, rows_per_proc * N, MPI_INT, 0, cart_comm);
    
    MPI_Cart_shift(cart_comm, 0, 1, &source_rank, &dest_rank);

    /* Ленточный алгоритм */
    for (int iter = 0; iter < size; iter++) {
        for (int i = 0; i < rows_per_proc; i++) {
            for (int j = 0; j < N; j++) {
                int sum = 0;
                for (int k = 0; k < rows_per_proc; k++) {
                    int current_b_block = (rank - iter + size) % size;
                    int col_A = current_b_block * rows_per_proc + k;
                    sum += A_local[i * N + col_A] * B_local[k * N + j];
                }
                C_local[i * N + j] += sum;
            }
        }

        if (iter < size - 1) {
            for (int i = 0; i < rows_per_proc * N; i++) {
                B_temp[i] = B_local[i];
            }

            MPI_Sendrecv(B_temp, rows_per_proc * N, MPI_INT, dest_rank, 0,
                        B_local, rows_per_proc * N, MPI_INT, source_rank, 0,
                        cart_comm, MPI_STATUS_IGNORE);
        }
    }

    MPI_Gather(C_local, rows_per_proc * N, MPI_INT,
               C, rows_per_proc * N, MPI_INT, 0, cart_comm);
    
    double end_time = MPI_Wtime();
    
    if (rank == 0) {
        if (N <= MAX_VISIBLE_SIZE) {
            print_matrix(C, N, N, "C (result)");
        }
        printf("Num of proc: %d | Execution time: %.6f seconds\n",
            size, end_time - start_time);
        
        free(A);
        free(B);
        free(C);
    }
    
    free(A_local);
    free(B_local);
    free(B_temp);
    free(C_local);
    
    MPI_Comm_free(&cart_comm);
    
    MPI_Finalize();
    return 0;
}

void init_matrix(int *matrix, int rows, int cols)
{
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (rand() % 21) - 10; 
    }
}

void print_matrix(int *matrix, int rows, int cols, char *name)
{
    printf("Matrix %s:\n", name);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%4d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}

void panic(char *message, MPI_Comm comm)
{
    int rank;
    MPI_Comm_rank(comm, &rank);
    if (rank == 0) {
        printf("Error: %s", message);
    }

    MPI_Abort(comm, 1);
}