#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) 
{
    int global_rank, global_size;
    int N;
    int dims[2], periods[2], coords[2];
    int remain_dims[2];
    int bcast_data;
    double start_time, end_time;
    MPI_Comm cart_comm, column_comm;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &global_size);

    if (global_rank == 0) {
        start_time = MPI_Wtime();
    }
    
    if (global_size % 3 != 0) {
        if (global_rank == 0) {
            printf("Error: Number of processes must be multiple of 3\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    N = global_size / 3;
    
    dims[0] = N;
    dims[1] = 3;
    periods[0] = 0;
    periods[1] = 0;

    int cart_rank;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cart_comm);
    MPI_Comm_rank(cart_comm, &cart_rank);
    MPI_Cart_coords(cart_comm, cart_rank, 2, coords);
    
    if (coords[0] == 0) {
        bcast_data = 100 * (coords[1] + 1);
    }
    
    remain_dims[0] = 1;
    remain_dims[1] = 0;
    
    MPI_Cart_sub(cart_comm, remain_dims, &column_comm);
    
    MPI_Bcast(&bcast_data, 1, MPI_INT, 0, column_comm);
    
    printf("Process %d (coords [%d,%d]) received data: %d\n", 
           global_rank, coords[0], coords[1], bcast_data);
    
    MPI_Comm_free(&column_comm);
    MPI_Comm_free(&cart_comm);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (global_rank == 0) {
        end_time = MPI_Wtime();
        printf("Num of proc: %d | Execution time: %.6f seconds",
                global_size, end_time - start_time);
    }

    MPI_Finalize();
    
    return 0;
}