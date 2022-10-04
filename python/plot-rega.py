#!/usr/bin/env python

import pandas as pd
import matplotlib.pyplot as plt

plt.rcParams["figure.figsize"] = [7.50, 3.50]
plt.rcParams["figure.autolayout"] = True

filename = 'measurments.csv'

# headers = ['Name', 'Age', 'Marks']
df = pd.read_csv(filename, low_memory=False) # 'marks.csv', names=headers)
x, y = df.Hum_1.min(), df.Hum_1.max()
df['H1S'] = (df.Hum_1 - x) / (y - x)
x, y = df.Temp_1.min(), df.Temp_1.max()
df['T1S'] = (df.Temp_1 - x) / (y - x)
x, y = df.Hum_2.min(), df.Hum_2.max()
#df['H2S'] = (df.Hum_2 - x) / (y - x)
df['H2S'] = df.Hum_2 / 8000.0
x, y = df.Temp_2.min(), df.Temp_2.max()
#df['T2S'] = (df.Temp_2 - x) / (y - x)
df['T2S'] = 500.0 / df.Temp_2
 # 'Hum_1', 'Temp_1', 'Hum_2', 'Temp_2'

df.plot(x='timestamp', y=['H1S', 'T1S', 'H2S','T2S'])
plt.show()

