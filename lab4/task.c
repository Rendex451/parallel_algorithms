#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    int world_rank, world_size;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    start_time = MPI_Wtime();

    double data[3];
    srand(time(NULL) + world_rank);
    for (int i = 0; i < 3; ++i) {
        data[i] = (double)(rand() % 1000) / 10.0;
    }

    printf("Process %d: data = [%.2f, %.2f, %.2f]\n", 
        world_rank, data[0], data[1], data[2]);

    MPI_Comm even_comm;
    int color = ((world_rank % 2 == 0) ? 0 : MPI_UNDEFINED);

    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &even_comm);

    if (color != MPI_UNDEFINED) {
        int even_rank, even_size;

        MPI_Comm_rank(even_comm, &even_rank);
        MPI_Comm_size(even_comm, &even_size);
        
        double min_values[3];

        MPI_Reduce(data, min_values, 3, MPI_DOUBLE, MPI_MIN, 0, even_comm);

        if (even_rank == 0) {
            end_time = MPI_Wtime();

            puts("============= Execution Results =============");
            printf("Min values: [%.2f, %.2f, %.2f]\n",
                min_values[0], min_values[1], min_values[2]);
            printf("Num of proc: %d | Execution time: %.6f seconds",
                world_size, end_time - start_time);
        }

        MPI_Comm_free(&even_comm);
    }

    MPI_Finalize();
    return 0;
}
