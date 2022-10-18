#!/usr/bin/env python3
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
#df['H0S'] = (df.Hum_0 - x) / (y - x)
df['1/H0S'] = 5.0 / (df.Hum_0 - 45.0)

x, y = df.Temp_0.min(), df.Temp_0.max()
# 569.0 888.12
df['1/T0S'] = 300.0 / (df.Temp_0 - 200.0)

x, y = df.Hum_LT.min(), df.Hum_LT.max()
 #df['H2S'] = (df.Hum_2 - x) / (y - x)
#53.6 63.6
df['HLT'] = df.Hum_LT / 60.0
x, y = df.Temp_LT.min(), df.Temp_LT.max()
#df['T2S'] = (df.Temp_2 - x) / (y - x)
df['1/TLT'] = 500.0 / df.Temp_LT
# 514.23 811.04

x, y = df.Hum_RB.min(), df.Hum_RB.max()
## 55.65 110.31
# 2858.07 7252.34
df['1/HRB'] = 3000.0 / (df.Hum_RB - 0)
x, y = df.Temp_RB.min(), df.Temp_RB.max()
#752.99 1023.72
df['1/TRB'] = 1000.0 / df.Temp_RB

x, y = df.Hum_RT.min(), df.Hum_RT.max()
# 83299.45  86559.42
df['HRT'] = (df.Hum_RT - 20000) / 60000.0


x, y = df.Temp_RT.min(), df.Temp_RT.max()
# 634.69 992.98
# 868.0 1034.0
df['1/TRT'] = 800.0 / df.Temp_RT

#df.plot(x='timestamp', y=['1/H0S', '1/T0S', 'HLT','1/TLT', '1/HRB','1/TRB'])
#df.plot(x='timestamp', y=['1/H0S', '1/T0S', 'HLT','1/TLT', '1/HRB','1/TRB', 'HRT','1/TRT'])
df.plot(x='timestamp', y=['1/H0S', '1/T0S', 'HLT','1/TLT', '1/TRB', 'HRT','1/TRT'])
plt.show()

