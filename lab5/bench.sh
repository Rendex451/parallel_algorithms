#!/bin/bash

OUTPUT_FILE="execution_times.csv"
TEMP_FILE="temp_output.txt"

# Создаем CSV файл с заголовком
echo "processes,execution_time" > $OUTPUT_FILE

for np in {3..24..3}; do
    echo "Запуск с $np процессами..."
    
    # Запускаем программу и сохраняем весь вывод во временный файл
    mpirun --oversubscribe -np $np ./task 2>&1 | tee "$TEMP_FILE"
    
    # Ищем строку с количеством процессов и временем выполнения
    # Используем grep для поиска строки, затем sed для извлечения чисел
    if grep -q "Num of proc.*Execution time" "$TEMP_FILE"; then
        # Извлекаем число процессов и время выполнения
        proc_time_line=$(grep "Num of proc.*Execution time" "$TEMP_FILE")
        
        # Извлекаем количество процессов (первое число в строке)
        proc_num=$(echo "$proc_time_line" | grep -oP 'Num of proc:\s*\K\d+')
        
        # Извлекаем время выполнения (число с плавающей точкой после "Execution time:")
        exec_time=$(echo "$proc_time_line" | grep -oP 'Execution time:\s*\K[0-9]+\.[0-9]+')
        
        # Записываем в CSV
        echo "$proc_num,$exec_time" >> $OUTPUT_FILE
        echo "Найдено: $proc_num процессов, время: $exec_time секунд"
    else
        echo "Строка с временем выполнения не найдена для $np процессов"
        # Можно записать данные из переменной цикла как запасной вариант
        echo "$np,N/A" >> $OUTPUT_FILE
    fi
    
    echo "---"
done

# Удаляем временный файл
rm -f "$TEMP_FILE"

echo "Результаты сохранены в $OUTPUT_FILE"
