import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Чтение CSV файла
df = pd.read_csv('execution_times.csv')

# Создание графика
plt.figure(figsize=(12, 8))

# Построение графика времени выполнения
plt.plot(df['processes'], df['execution_time'], 'bo-', linewidth=2, markersize=6, label='Время выполнения')
plt.xlabel('Количество процессов')
plt.ylabel('Время выполнения (секунды)')
plt.title('Зависимость времени выполнения от количества процессов')
plt.grid(True, alpha=0.3)
plt.legend()
plt.savefig('lab4_execution_time_plot.png', dpi=300, bbox_inches='tight')
plt.show()
