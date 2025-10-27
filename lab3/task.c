#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define ROWS 10
#define COLS 10

void init_matrix(int *matrix, int rows, int cols)
{
    for (int i = 0; i < rows * cols; ++i) {
        matrix[i] = rand() % 100 + 1;
    }
}

void print_matrix(int *matrix, int rows, int cols)
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("%3d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    int rank, size;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int total_rows = ROWS, total_cols = COLS;
    int total_elements = total_rows * total_cols;
    
    int rows_per_proc = total_rows / size;
    int rem = total_rows % size;
    
    int local_rows;
    if (rank < rem) {
        local_rows = rows_per_proc + 1;
    } else {
        local_rows = rows_per_proc;
    }
    
    int local_elements = local_rows * total_cols;
    
    int *local_matrix = (int*)malloc(local_elements * sizeof(int));
    int *full_matrix = NULL;
    
    if (rank == 0) {
        full_matrix = (int*)malloc(total_elements * sizeof(int));
        srand(time(NULL));
        init_matrix(full_matrix, total_rows, total_cols);
        
        puts("============= Input Matrix =============");
        print_matrix(full_matrix, total_rows, total_cols);
        
        start_time = MPI_Wtime();
        
        int *sendcounts = (int*)malloc(size * sizeof(int));
        int *displs = (int*)malloc(size * sizeof(int));
        
        int offset = 0;
        for (int i = 0; i < size; ++i) {
            int rows_for_i = (i < rem) ? rows_per_proc + 1 : rows_per_proc;
            sendcounts[i] = rows_for_i * total_cols;
            displs[i] = offset * total_cols;
            offset += rows_for_i;
        }
        
        MPI_Scatterv(full_matrix, sendcounts, displs, MPI_INT,
                    local_matrix, local_elements, MPI_INT,
                    0, MPI_COMM_WORLD);
        
        free(sendcounts);
        free(displs);
        
    } else {
        start_time = MPI_Wtime();
        MPI_Scatterv(NULL, NULL, NULL, MPI_INT,
                    local_matrix, local_elements, MPI_INT,
                    0, MPI_COMM_WORLD);
    }
    
    int local_sum = 0;
    for (int i = 0; i < local_elements; ++i) {
        local_sum += local_matrix[i];
    }
    
    int global_sum;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, 
        MPI_SUM, 0, MPI_COMM_WORLD);
    
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        puts("============= Execution Results =============");
        printf("Total sum: %d\n", global_sum);
        printf("Num of proc: %d | Execution time: %.6f seconds",
                size, end_time - start_time);
        
        free(full_matrix);
    }
    
    free(local_matrix);
    MPI_Finalize();
    
    return 0;
}