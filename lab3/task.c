#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define ROWS 100
#define COLS 100

int** allocate_matrix(int rows, int cols) {
    int **matrix = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*)malloc(cols * sizeof(int));
    }
    return matrix;
}

void free_matrix(int **matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void fill_matrix(int **matrix, int rows, int cols) {
    srand(clock());
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            matrix[i][j] = rand() % 100;
}

int main(int argc, char *argv[]) {
    int rank, size;
    int rows, cols;
    int **matrix;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    rows = ROWS;
    cols = COLS;

    if (rank == 0) {
        matrix = allocate_matrix(rows, cols);
        fill_matrix(matrix, rows, cols);
    }

    int rows_per_proc = rows / size;
    int rem = rows % size;

    int local_rows;
    if (rank < rem) {
        local_rows = rows_per_proc + 1;
    } else {
        local_rows = rows_per_proc;
    }

        // Выделяем память для локальной части матрицы
    int **local_matrix = allocate_matrix(local_rows, cols);
    
    // Создаем массивы для MPI_Scatterv
    int *sendcounts = NULL;
    int *displs = NULL;
    
    if (rank == 0) {
        sendcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        
        int offset = 0;
        for (int i = 0; i < size; i++) {
            int rows_for_i = (i < remainder) ? rows_per_process + 1 : rows_per_process;
            sendcounts[i] = rows_for_i * cols;
            displs[i] = offset;
            offset += sendcounts[i];
        }
        
        // Создаем одномерный буфер для рассылки
        int *send_buffer = (int*)malloc(rows * cols * sizeof(int));
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                send_buffer[i * cols + j] = matrix[i][j];
            }
        }
        
        // Рассылаем данные
        MPI_Scatterv(send_buffer, sendcounts, displs, MPI_INT,
                    local_matrix[0], local_rows * cols, MPI_INT,
                    0, MPI_COMM_WORLD);
        
        free(send_buffer);
    } else {
        // Принимаем свою часть данных
        MPI_Scatterv(NULL, NULL, NULL, MPI_INT,
                    local_matrix[0], local_rows * cols, MPI_INT,
                    0, MPI_COMM_WORLD);
    }
    
    // Синхронизация перед началом параллельного вычисления
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    
    // Вычисляем локальную сумму
    long long local_sum = 0;
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < cols; j++) {
            local_sum += local_matrix[i][j];
        }
    }
    
    // Собираем все локальные суммы
    long long global_sum = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    
    end_time = MPI_Wtime();
    
    // Выводим результаты
    if (rank == 0) {
        printf("Параллельная сумма: %lld, время: %.6f сек\n", 
               global_sum, end_time - start_time);
        printf("Разница: %lld\n", llabs(seq_sum - global_sum));
        
        // Освобождаем память
        free_matrix(matrix, rows);
        free(sendcounts);
        free(displs);
    }
    
    // Освобождаем локальную память
    free_matrix(local_matrix, local_rows);
    
    MPI_Finalize();
    return 0;
}
