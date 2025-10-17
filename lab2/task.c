#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>

#define BUFF_SIZE 1000000

#define ERR_MALLOC 1
#define ERR_SEND_DATA 2
#define ERR_RECV_RESULT 3
#define ERR_RECV_DATA 4
#define ERR_SEND_RESULT 5

int count_zeroes(int *arr, int size);

int main(int argc, char **argv)
{
    int rank, size;
    int success;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        double start_time, end_time, exec_time;

        start_time = MPI_Wtime();

        int *arr = malloc(BUFF_SIZE * sizeof(int));
        if (!arr) MPI_Abort(MPI_COMM_WORLD, 1);

        srand(time(NULL));
        puts("Initial array:");
        for (int i = 0; i < BUFF_SIZE; ++i) {
            arr[i] = rand() % 7;
            printf("%d ", arr[i]);
        }
        printf("\n");
        
        int elements_per_proc = BUFF_SIZE / size;
        int remainder = BUFF_SIZE % size;
        
        int start_index = elements_per_proc + (0 < remainder ? 1 : 0);
        for (int i = 1; i < size; ++i) {
            int proc_elements = elements_per_proc + (i < remainder ? 1 : 0);
            if (MPI_Send(&proc_elements,1, MPI_INT, i, 0, MPI_COMM_WORLD) 
                    != MPI_SUCCESS) {
                free(arr); MPI_Abort(MPI_COMM_WORLD, ERR_SEND_DATA);
            };
            if (MPI_Send(&arr[start_index], proc_elements, MPI_INT, i, 0, MPI_COMM_WORLD) 
                    != MPI_SUCCESS) {
                free(arr); MPI_Abort(MPI_COMM_WORLD, ERR_SEND_DATA);
            }
            start_index += proc_elements;
        }

        int local_slice_size = elements_per_proc + (0 < remainder ? 1 : 0);
        int local_zeros = count_zeroes(arr, local_slice_size);
        printf("Process %d: zeros = %d\n", rank, local_zeros);

        int total_zeros = local_zeros;
        for (int i = 1; i < size; ++i) {
            int proc_zeroes;
            if (MPI_Recv(&proc_zeroes, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status) 
                    != MPI_SUCCESS) {
                free(arr); MPI_Abort(MPI_COMM_WORLD, ERR_RECV_RESULT);
            }
            total_zeros += proc_zeroes;
        }

        end_time = MPI_Wtime();
        exec_time = end_time - start_time;

        puts("============= Execution Results =============");
        printf("Total zeroes: %d\n", total_zeros);
        printf("Num of proc: %d | Execution time: %.6f seconds",
                size, exec_time);

        free(arr);
    } else {
        int local_slice_size;
        if (MPI_Recv(&local_slice_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status) 
                != MPI_SUCCESS) {
            MPI_Abort(MPI_COMM_WORLD, ERR_RECV_DATA);
        }
        
        int *local_arr = malloc(local_slice_size * sizeof(int));
        if (!local_arr) {
            MPI_Abort(MPI_COMM_WORLD, ERR_MALLOC);
        }
        if (MPI_Recv(local_arr, local_slice_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status)
                != MPI_SUCCESS) {
            free(local_arr); MPI_Abort(MPI_COMM_WORLD, ERR_RECV_DATA);
        }

        int local_zeros = count_zeroes(local_arr, local_slice_size); 
        printf("Process %d: zeros = %d\n", rank, local_zeros);
        if (MPI_Send(&local_zeros, 1, MPI_INT, 0, 2, MPI_COMM_WORLD) 
                != MPI_SUCCESS) {
            free(local_arr); MPI_Abort(MPI_COMM_WORLD, ERR_SEND_RESULT);
        }

        free(local_arr);
    }

    MPI_Finalize();
    return 0;
}

int count_zeroes(int *arr, int size)
{
    int zeros_count = 0;
    for (int i = 0; i < size; ++i) {
        if (arr[i] == 0)
            zeros_count++;
    }
    return zeros_count;
}
