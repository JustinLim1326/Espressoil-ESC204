# -*- coding:utf-8 -*
import busio
import digitalio
import board
import time

uart = busio.UART(board.GP4, None, baudrate=115200)

while True:
    uart.write("Hello Fellow Citizens*".encode("ascii", "ignore"))
    #print("hi")
