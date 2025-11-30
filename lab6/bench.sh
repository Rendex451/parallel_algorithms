#!/bin/bash

OUTPUT_FILE="execution_times.csv"
TEMP_FILE="temp_output.txt"

if [ $# -lt 1 ]; then
    echo "Usage: $0 <matrix_size>"
    exit 1
fi

MATRIX_SIZE=$1
echo "processes,execution_time" > $OUTPUT_FILE

for np in {2,5,10,25,50}; do
    echo "Запуск с $np процессами..."
    
    mpirun --oversubscribe -np $np ./parallel $MATRIX_SIZE 2>&1 | tee "$TEMP_FILE"
    
    if grep -q "Num of proc.*Execution time" "$TEMP_FILE"; then
        proc_time_line=$(grep "Num of proc.*Execution time" "$TEMP_FILE")
        proc_num=$(echo "$proc_time_line" | grep -oP 'Num of proc:\s*\K\d+')
        exec_time=$(echo "$proc_time_line" | grep -oP 'Execution time:\s*\K[0-9]+\.[0-9]+')
        echo "$proc_num,$exec_time" >> $OUTPUT_FILE
        echo "Найдено: $proc_num процессов, время: $exec_time секунд"
    else
        echo "Строка с временем выполнения не найдена для $np процессов"
        echo "$np,N/A" >> $OUTPUT_FILE
    fi
    
    echo "---"
done

rm -f "$TEMP_FILE"

echo "Результаты сохранены в $OUTPUT_FILE"
