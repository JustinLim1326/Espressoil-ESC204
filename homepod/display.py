
'''
ESC204 2024S Lab 1 Task D
Task: Light up external LED on button press.
'''
# Import libraries needed for blinking the LED
import board
import digitalio
import time

print("imported")

D_I = digitalio.DigitalInOut(board.GP18)
D_I.direction = digitalio.Direction.OUTPUT

R_W = digitalio.DigitalInOut(board.GP19)
R_W.direction = digitalio.Direction.OUTPUT

E = digitalio.DigitalInOut(board.GP20)
E.direction = digitalio.Direction.OUTPUT

P1 = digitalio.DigitalInOut(board.GP2)
P1.direction = digitalio.Direction.OUTPUT

P2 = digitalio.DigitalInOut(board.GP3)
P2.direction = digitalio.Direction.OUTPUT

P3 = digitalio.DigitalInOut(board.GP4)
P3.direction = digitalio.Direction.OUTPUT

P4 = digitalio.DigitalInOut(board.GP5)
P4.direction = digitalio.Direction.OUTPUT

P5 = digitalio.DigitalInOut(board.GP6)
P5.direction = digitalio.Direction.OUTPUT

P6 = digitalio.DigitalInOut(board.GP7)
P6.direction = digitalio.Direction.OUTPUT

P7 = digitalio.DigitalInOut(board.GP8)
P7.direction = digitalio.Direction.OUTPUT

P8 = digitalio.DigitalInOut(board.GP9)
P8.direction = digitalio.Direction.OUTPUT

def turnOn():
    E.value = False
    time.sleep(1)
    command(0x30)
    time.sleep(1)
    command(0x30)
    time.sleep(1)
    command(0x30)
    time.sleep(1)
    command(0x38)
    command(0x10)
    command(0x0c)
    command(0x06)
    
def write(i):
    P1.value = i
    D_I.value = True
    R_W.value = False
    E.value = True
    time.sleep(0.01)
    E.value = False
    
def command(i):
    P1.value = i
    D_I.value = False
    R_W.value = False
    E.value = True
    time.sleep(0.01)
    E.value = False
    
turnOn()
print("written")
