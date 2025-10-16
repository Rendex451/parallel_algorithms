#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "mpi.h"

#define FIRST_THREAD 0

int* get_interval(int, int, int*);
void print_simple_range(int, int);
void wait(int);

int main(int argc, char **argv)
{
	// инициализируем необходимые переменные
	int thread, thread_size, processor_name_length;
	int *thread_range, *interval;
	double cpu_time_start, cpu_time_fini;
	char *processor_name = (char*)malloc(MPI_MAX_PROCESSOR_NAME * sizeof(char));

	MPI_Status status;
	interval = (int*)malloc(2 * sizeof(int));
	
	// Инициализируем работу MPI
	MPI_Init(&argc, &argv);
	
	// Получаем имя физического процессора
	MPI_Get_processor_name(processor_name, &processor_name_length);
	
	// Получаем номер конкретного процесса на котором запущена программа
	MPI_Comm_rank(MPI_COMM_WORLD, &thread);
	
	// Получаем количество запущенных процессов
	MPI_Comm_size(MPI_COMM_WORLD, &thread_size);
	
	// Если это первый процесс, то выполняем следующий участок кода
	if (thread == FIRST_THREAD) {
		// Выводим информацию о запуске
		printf("----- Programm information -----");
		printf(">>> Processor: %s\n", processor_name);
		printf(">>> Num threads: %d\n", thread_size);
		printf(">>> Input the interval: ");

		// Просим пользователья ввести интервал на котором будут вычисления
		scanf("%d %d", &interval[0], &interval[1]);

		// Каждому процессу отправляем полученный интервал с тегом сообщения 0. 
		for (int to_thread = 1; to_thread < thread_size; ++to_thread) {
            MPI_Send(&interval, 2, MPI_INT, to_thread, 0, MPI_COMM_WORLD);
        }

		// Начинаем считать время выполнения
		cpu_time_start = MPI_Wtime();
	} else {
        MPI_Recv(&interval, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);    
    }

	// Все процессы запрашивают свой интервал
	thread_range = get_interval(thread, thread_size, interval);

	// После чего отправляют полученный интервал в функцию которая производит вычисления
	print_simple_range(thread_range[0], thread_range[1]);

	// Последний процесс фиксирует время завершения, ожидает 1 секунду и выводит результат
	if (thread == thread_size - 1) {
		cpu_time_fini = MPI_Wtime();
		wait(1);
		printf("CPU Time: %lf ms\n", (cpu_time_fini - cpu_time_start) * 1000);
	}

    free(processor_name);
    free(interval);
    free(thread_range);

	MPI_Finalize();
    return 0;
}

int* get_interval(int proc, int size, int* interval)
{
	// Функция для рассчета интервала каждого процесса
	int* range = (int*)malloc(2 * sizeof(int));
	int interval_size = (interval[1] - interval[0]) / size;

	range[0] = interval[0] + interval_size * proc;
	range[1] = interval[0] + interval_size * (proc + 1);
	range[1] = range[1] == interval[1] - 1 ? interval[1] : range[1];

	return range;
}

void print_simple_range(int ibeg, int iend)
{
    // Простейшая реализация определения простого числа
    int res;
    for (int i = ibeg; i <= iend; i++)
    {
        res = 1;
        while (res) {
            res = 0;
            for (int j = 2; j < i; j++) 
                if (i % j == 0) 
                    res = 1;
            if (res) 
                break;
        }

        res = !res;
        if (res) 
            printf("Simple value ---> %d\n", i);
    }
}

void wait(int seconds)
{
 	// Функция ожидающая в течение seconds секунд
	clock_t endwait;
	endwait = clock () + seconds * CLOCKS_PER_SEC ;
	while (clock() < endwait) {};
}
