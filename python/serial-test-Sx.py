#!/usr/bin/python3
# -*- coding: utf-8 -*-
# vim: set ts=4 sw=4 tw=0 et :
# https://github.com/ikalchev/py-sds011
########################################################
__author__ = "Bernardo Carvalho <bernardo.carvalho@tecnico.ulisboa.pt>"
__license__ = "GPL3"
__version__ = "1.0"

import serial
import csv, time, datetime


#datafile=open(filename, 'a')

ser1 = serial.Serial('/dev/ttyS1', baudrate = 115200, parity=serial.PARITY_NONE, \
        stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS)
ser1.flushInput()
ser2 = serial.Serial('/dev/ttyS2', baudrate = 115200, parity=serial.PARITY_NONE, \
        stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS)
ser2.flushInput()

msg = "ZANE:1:00004:XX_X.X_XXXX_000XX:\r\n"
try:
    while True:
        ser1.write(msg.encode())
        ser2.write(msg.encode())
        time.sleep(1)
    
except KeyboardInterrupt:
    ser1.close()
    ser2.close()
    print("Keyboard Interrupt")
