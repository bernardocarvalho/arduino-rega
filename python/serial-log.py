#!/usr/bin/python3
# -*- coding: utf-8 -*-
# vim: set ts=4 sw=4 tw=0 et :
# https://github.com/ikalchev/py-sds011
########################################################
__author__ = "Bernardo Carvalho <bernardo.carvalho@tecnico.ulisboa.pt>"
__license__ = "GPL3"
__version__ = "1.0"

import serial
import time
import csv, time, datetime


#datafile=open(filename, 'a')

ser = serial.Serial('/dev/ttyUSB0', baudrate = 115200, parity=serial.PARITY_NONE, \
        stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS)
ser.flushInput()

try:
    with open("measurments.csv","a") as csvfile:
        log = csv.writer(csvfile, delimiter=",",quotechar=" ", quoting=csv.QUOTE_MINIMAL)
        logcols = ["timestamp       ","sec","Hum 1","Temp 1"," Hum 2","Temp 2","H20 Meas","H2O Pump"]
        log.writerow(logcols)
        while True:
            ser_bytes = ser.readline()
            str_line = ser_bytes[0:len(ser_bytes)-2].decode("utf-8") # strip \r\n
            now = datetime.datetime.now()
        #decoded_bytes = float(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))
            vals = [str(now)] + [str(str_line)] 
            log.writerow(vals)
            csvfile.flush()
            print(vals)
        #datafile.write(str(time.time()) + str(ser_line))
    
except KeyboardInterrupt:
    ser.close()
    print("Keyboard Interrupt")
