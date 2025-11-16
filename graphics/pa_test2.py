import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('execution_times.csv')

process_col = df.columns[0]
time_col = df.columns[1]

df = df.sort_values(by=process_col)

T0 = df[time_col].iloc[0]  # время на 1 процессе
df['speedup'] = T0 / df[time_col]

# Простой график
plt.figure(figsize=(10, 6))
plt.plot(df[process_col], df['speedup'], 'bo-', linewidth=2, markersize=8)
plt.xlabel('Количество процессов')
plt.ylabel('Ускорение f = T₀/T_seq')
plt.title('Ускорение программы')
plt.grid(True)

plt.tight_layout()
plt.savefig('lab4_speedup.png', dpi=300)
plt.show()
