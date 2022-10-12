#!/usr/bin/env python
# vim: sta:et:sw=4:ts=4:sts=4

import sys
import pandas as pd
import matplotlib.pyplot as plt

plt.rcParams["figure.figsize"] = [7.50, 3.50]
plt.rcParams["figure.autolayout"] = True

if len(sys.argv) > 1:
    filename = str(sys.argv[1])
else:
    filename = 'rega-log.csv'

# headers = ['Name', 'Age', 'Marks']
#  Hum_0', ' Temp0', '  Hum_LT', ' Temp_LT',
#       '  Hum_RB', ' Temp_RB', ' H20_Meas', ' H2O_Pump '],
df = pd.read_csv(filename, low_memory=False) # 'marks.csv', names=headers)
x, y = df.Hum_0.min(), df.Hum_0.max()
# 50.31 51.8
df['H0S'] = (df.Hum_0 - x) / (y - x)

x, y = df.Temp_0.min(), df.Temp_0.max()
# 569.0 888.12
df['T0S'] = (df.Temp_0 - x) / (y - x)

x, y = df.Hum_LT.min(), df.Hum_LT.max()
#df['H2S'] = (df.Hum_2 - x) / (y - x)
df['HLT'] = df.Hum_LT / 8000.0
x, y = df.Temp_LT.min(), df.Temp_LT.max()
# 3190.79 7340.12

#df['T2S'] = (df.Temp_2 - x) / (y - x)
df['TLT'] = 500.0 / df.Temp_LT
x, y = df.Hum_RB.min(), df.Hum_RB.max()
# 514.23 811.04
df['HRB'] = 55.0 / df.Hum_RB
x, y = df.Temp_RB.min(), df.Temp_RB.max()
# 55.65 110.31
df['TRB'] = 700.0 / df.Temp_RB
#752.99 1023.72

df.plot(x='timestamp', y=['H0S', 'T0S', 'HLT','TLT', 'HRB','TRB'])
plt.show()

