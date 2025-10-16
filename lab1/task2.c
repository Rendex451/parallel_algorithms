#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Status status;   
    
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (rank == 0) {
        double start_time, end_time;
        start_time = MPI_Wtime();

        printf("Process 0: my rank is %d\n", rank);
        
        for (int i = 1; i < size; i++) {
            int received_rank;
            MPI_Recv(&received_rank, 
                    1, 
                    MPI_INT, 
                    i,
                    0, 
                    MPI_COMM_WORLD, 
                    &status);
            
            printf("Process 0: received rank %d from process %d\n", 
                    received_rank, status.MPI_SOURCE);
        }

        end_time = MPI_Wtime();
        double exec_time = end_time - start_time;

        puts("============= Execution Results =============");
        printf("Num of proc: %d | Execution time: %.6f seconds",
                size, exec_time);

    }
    else {
        MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    
    MPI_Finalize();
    return 0;
}
