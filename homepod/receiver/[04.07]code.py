from digitalio import DigitalInOut, Direction, Pull
import digitalio
import time
import board
import busio

#LCD Address
class LcdApi:
    LCD_CLR = 0x01  # DB0: clear display
    LCD_HOME = 0x02  # DB1: return to home position

    LCD_ENTRY_MODE = 0x04  # DB2: set entry mode
    LCD_ENTRY_INC = 0x02  # --DB1: increment
    LCD_ENTRY_SHIFT = 0x01  # --DB0: shift

    LCD_ON_CTRL = 0x08  # DB3: turn lcd/cursor on
    LCD_ON_DISPLAY = 0x04  # --DB2: turn display on
    LCD_ON_CURSOR = 0x02  # --DB1: turn cursor on
    LCD_ON_BLINK = 0x01  # --DB0: blinking cursor

    LCD_MOVE = 0x10  # DB4: move cursor/display
    LCD_MOVE_DISP = 0x08  # --DB3: move display (0-> move cursor)
    LCD_MOVE_RIGHT = 0x04  # --DB2: move right (0-> left)

    LCD_FUNCTION = 0x20  # DB5: function set
    LCD_FUNCTION_8BIT = 0x10  # --DB4: set 8BIT mode (0->4BIT mode)
    LCD_FUNCTION_2LINES = 0x08  # --DB3: two lines (0->one line)
    LCD_FUNCTION_10DOTS = 0x04  # --DB2: 5x10 font (0->5x7 font)
    LCD_FUNCTION_RESET = 0x30  # See "Initializing by Instruction" section

    LCD_CGRAM = 0x40  # DB6: set CG RAM address
    LCD_DDRAM = 0x80  # DB7: set DD RAM address

    LCD_RS_CMD = 0
    LCD_RS_DATA = 1

    LCD_RW_WRITE = 0
    LCD_RW_READ = 1

    def __init__(self, num_lines, num_columns):
        self.num_lines = num_lines
        if self.num_lines > 4:
            self.num_lines = 4
        self.num_columns = num_columns
        if self.num_columns > 40:
            self.num_columns = 40
        self.cursor_x = 0
        self.cursor_y = 0
        self.implied_newline = False
        self.backlight = True
        self.display_off()
        self.backlight_on()
        self.clear()
        self.hal_write_command(self.LCD_ENTRY_MODE | self.LCD_ENTRY_INC)
        self.hide_cursor()
        self.display_on()

    def clear(self):
        """Clears the LCD display and moves the cursor to the top left
        corner.
        """
        self.hal_write_command(self.LCD_CLR)
        self.hal_write_command(self.LCD_HOME)
        self.cursor_x = 0
        self.cursor_y = 0

    def show_cursor(self):
        """Causes the cursor to be made visible."""
        self.hal_write_command(
            self.LCD_ON_CTRL | self.LCD_ON_DISPLAY | self.LCD_ON_CURSOR
        )

    def hide_cursor(self):
        """Causes the cursor to be hidden."""
        self.hal_write_command(self.LCD_ON_CTRL | self.LCD_ON_DISPLAY)

    def blink_cursor_on(self):
        """Turns on the cursor, and makes it blink."""
        self.hal_write_command(
            self.LCD_ON_CTRL
            | self.LCD_ON_DISPLAY
            | self.LCD_ON_CURSOR
            | self.LCD_ON_BLINK
        )

    def blink_cursor_off(self):
        """Turns on the cursor, and makes it no blink (i.e. be solid)."""
        self.hal_write_command(
            self.LCD_ON_CTRL | self.LCD_ON_DISPLAY | self.LCD_ON_CURSOR
        )

    def display_on(self):
        """Turns on (i.e. unblanks) the LCD."""
        self.hal_write_command(self.LCD_ON_CTRL | self.LCD_ON_DISPLAY)

    def display_off(self):
        """Turns off (i.e. blanks) the LCD."""
        self.hal_write_command(self.LCD_ON_CTRL)

    def backlight_on(self):
        """Turns the backlight on.
        This isn't really an LCD command, but some modules have backlight
        controls, so this allows the hal to pass through the command.
        """
        self.backlight = True
        self.hal_backlight_on()

    def backlight_off(self):
        """Turns the backlight off.
        This isn't really an LCD command, but some modules have backlight
        controls, so this allows the hal to pass through the command.
        """
        self.backlight = False
        self.hal_backlight_off()

    def move_to(self, cursor_x, cursor_y):
        """Moves the cursor position to the indicated position. The cursor
        position is zero based (i.e. cursor_x == 0 indicates first column).
        """
        self.cursor_x = cursor_x
        self.cursor_y = cursor_y
        addr = cursor_x & 0x3F
        if cursor_y & 1:
            addr += 0x40  # Lines 1 & 3 add 0x40
        if cursor_y & 2:  # Lines 2 & 3 add number of columns
            addr += self.num_columns
        self.hal_write_command(self.LCD_DDRAM | addr)

    def putchar(self, char):
        """Writes the indicated character to the LCD at the current cursor
        position, and advances the cursor by one position.
        """
        if char == "\n":
            if self.implied_newline:
                # self.implied_newline means we advanced due to a wraparound,
                # so if we get a newline right after that we ignore it.
                pass
            else:
                self.cursor_x = self.num_columns
        else:
            self.hal_write_data(ord(char))
            self.cursor_x += 1
        if self.cursor_x >= self.num_columns:
            self.cursor_x = 0
            self.cursor_y += 1
            self.implied_newline = char != "\n"
        if self.cursor_y >= self.num_lines:
            self.cursor_y = 0
        self.move_to(self.cursor_x, self.cursor_y)

    def putstr(self, string):
        """Write the indicated string to the LCD at the current cursor
        position and advances the cursor position appropriately.
        """
        for char in string:
            self.putchar(char)

    def custom_char(self, location, charmap):
        """Write a character to one of the 8 CGRAM locations, available
        as chr(0) through chr(7).
        """
        location &= 0x7
        self.hal_write_command(self.LCD_CGRAM | (location << 3))
        self.hal_sleep_us(40)
        for i in range(8):
            self.hal_write_data(charmap[i])
            self.hal_sleep_us(40)
        self.move_to(self.cursor_x, self.cursor_y)

    def hal_backlight_on(self):
        """Allows the hal layer to turn the backlight on.
        If desired, a derived HAL class will implement this function.
        """
        print("hal backlight on")
        pass

    def hal_backlight_off(self):
        """Allows the hal layer to turn the backlight off.
        If desired, a derived HAL class will implement this function.
        """
        print("hal backlight off")
        pass

    def hal_write_command(self, cmd):
        """Write a command to the LCD.
        It is expected that a derived HAL class will implement this
        function.
        """
        print("hal write comm")
        raise NotImplementedError

    def hal_write_data(self, data):
        """Write data to the LCD.
        It is expected that a derived HAL class will implement this
        function.
        """
        print("hal write data")
        raise NotImplementedError

    def hal_sleep_us(self, usecs):
        """Sleep for some time (given in microseconds)."""
        time.sleep(usecs / 1000000)

#LCD Functions
class GpioLcd(LcdApi):
    def __init__(
        self,
        rs_pin,
        enable_pin,
        d0_pin=None,
        d1_pin=None,
        d2_pin=None,
        d3_pin=None,
        d4_pin=None,
        d5_pin=None,
        d6_pin=None,
        d7_pin=None,
        rw_pin=None,
        backlight_pin=None,
        num_lines=2,
        num_columns=16,
    ):
        self.rs_pin = rs_pin
        self.enable_pin = enable_pin
        self.rw_pin = rw_pin
        self.backlight_pin = backlight_pin
        self._4bit = True
        self.d0_pin = d0_pin
        self.d1_pin = d1_pin
        self.d2_pin = d2_pin
        self.d3_pin = d3_pin
        self.d4_pin = d4_pin
        self.d5_pin = d5_pin
        self.d6_pin = d6_pin
        self.d7_pin = d7_pin
        if self.d0_pin and self.d1_pin and self.d2_pin and self.d3_pin:
            self._4bit = False
        self.rs_pin = digitalio.DigitalInOut(rs_pin)
        self.rs_pin.direction = digitalio.Direction.OUTPUT
        self.rs_pin.value = False
        if self.rw_pin:
            self.rw_pin = digitalio.DigitalInOut(rw_pin)
            self.rw_pin.direction = digitalio.Direction.OUTPUT
            self.rw_pin.value = False
        self.enable_pin = digitalio.DigitalInOut(enable_pin)
        self.enable_pin.direction = digitalio.Direction.OUTPUT
        self.enable_pin.value = False

        self.d4_pin = digitalio.DigitalInOut(d4_pin)
        self.d4_pin.direction = digitalio.Direction.OUTPUT
        self.d4_pin.value = False

        self.d5_pin = digitalio.DigitalInOut(d5_pin)
        self.d5_pin.direction = digitalio.Direction.OUTPUT
        self.d5_pin.value = False

        self.d6_pin = digitalio.DigitalInOut(d6_pin)
        self.d6_pin.direction = digitalio.Direction.OUTPUT
        self.d6_pin.value = False

        self.d7_pin = digitalio.DigitalInOut(d7_pin)
        self.d7_pin.direction = digitalio.Direction.OUTPUT
        self.d7_pin.value = False

        if not self._4bit:
            self.d0_pin = digitalio.DigitalInOut(d0_pin)
            self.d0_pin.direction = digitalio.Direction.OUTPUT
            self.d0_pin.value = False

            self.d1_pin = digitalio.DigitalInOut(d1_pin)
            self.d1_pin.direction = digitalio.Direction.OUTPUT
            self.d1_pin.value = False

            self.d2_pin = digitalio.DigitalInOut(d2_pin)
            self.d2_pin.direction = digitalio.Direction.OUTPUT
            self.d2_pin.value = False

            self.d3_pin = digitalio.DigitalInOut(d3_pin)
            self.d3_pin.direction = digitalio.Direction.OUTPUT
            self.d3_pin.value = False
        if self.backlight_pin is not None:
            self.backlight_pin = digitalio.DigitalInOut(backlight_pin)
            self.backlight_pin.direction = digitalio.Direction.OUTPUT
            self.backlight_pin.value = False

        # See about splitting this into begin

        time.sleep(20 / 1000)
        # Send reset 3 times
        self.hal_write_init_nibble(self.LCD_FUNCTION_RESET)
        time.sleep(5 / 1000)
        self.hal_write_init_nibble(self.LCD_FUNCTION_RESET)
        time.sleep(1 / 1000)
        self.hal_write_init_nibble(self.LCD_FUNCTION_RESET)
        time.sleep(1 / 1000)
        cmd = self.LCD_FUNCTION
        if not self._4bit:
            cmd |= self.LCD_FUNCTION_8BIT
        self.hal_write_init_nibble(cmd)
        time.sleep(1 / 1000)
        LcdApi.__init__(self, num_lines, num_columns)
        if num_lines > 1:
            cmd |= self.LCD_FUNCTION_2LINES
        self.hal_write_command(cmd)

    def hal_pulse_enable(self):
        self.enable_pin.value = 0
        time.sleep(1 / 1000000)
        self.enable_pin.value = 1
        time.sleep(1 / 1000000)
        self.enable_pin.value = 0
        time.sleep(1 / 1000000)

    def hal_write_init_nibble(self, nibble):
        self.hal_write_4bits(nibble >> 4)

    def hal_backlight_on(self):
        if self.backlight_pin:
            self.backlight_pin.value = 1

    def hal_backlight_off(self):
        if self.backlight_pin:
            self.backlight_pin.value = 0

    def hal_write_command(self, cmd):
        self.rs_pin.value = 0
        self.hal_write_8bits(cmd)
        if cmd <= 3:
            # The home and clear commands require a worst
            # case delay of 4.1 msec
            time.sleep(5 / 1000)

    def hal_write_data(self, data):
        self.rs_pin.value = 1
        self.hal_write_8bits(data)

    def hal_write_8bits(self, value):
        if self.rw_pin:
            self.rw_pin.value = 0
        if self._4bit:
            self.hal_write_4bits(value >> 4)
            self.hal_write_4bits(value)
        else:
            self.d3_pin.value = value & 0x08
            self.d2_pin.value = value & 0x04
            self.d1_pin.value = value & 0x02
            self.d0_pin.value = value & 0x01
            self.hal_write_4bits(value >> 4)

    def hal_write_4bits(self, nibble):
        self.d7_pin.value = nibble & 0x08
        self.d6_pin.value = nibble & 0x04
        self.d5_pin.value = nibble & 0x02
        self.d4_pin.value = nibble & 0x01
        self.hal_pulse_enable()

#The ljust() method returns a left-justified string of a specified minimum width.
def ljust(string, width, fillchar=" "):
    if len(string) >= width:
        return string
    else:
        return string + fillchar * (width - len(string))

#Connect Raspberry Pi Pico to Arduino Nano
uart = busio.UART(None, board.GP1, baudrate=115200)

#Initialize LCD with approprirate pins
lcd = GpioLcd(
    rs_pin=board.GP14,
    enable_pin=board.GP15,
    d0_pin=board.GP6,
    d1_pin=board.GP7,
    d2_pin=board.GP8,
    d3_pin=board.GP9,
    d4_pin=board.GP10,
    d5_pin=board.GP11,
    d6_pin=board.GP12,
    d7_pin=board.GP13,
    num_lines=2,
    num_columns=16,
)


x_offset = 0    #Initialize the scrolling effect
space = 3       #Space between the two strings
m = 0           #Index for the string list for different reccomendations
str_list = []   #List of string recommendations
cor_list= []    #List of str_list for correcting inaccurate strings

#Default LCD Display
received_str = "Nothing Received;       ^_^/Please turn on;the node sensor"

while True:
    for k in range(max(n, 10)):
        if uart.in_waiting > 0: #Check if data is available to read
            received_data = uart.readline()
            if received_data:
                received_str = received_data.decode().strip()
                print("Received data:", received_str)

        #Split reccomenations into seperate strings and return as list
        str_list = received_str.split("/")
        
        #Correcting the inaccurate strings
        cor_list.append(str_list)
        if len(cor_list)>5:
            if str_list not in cor_list[:-1]:
                str_list = cor_list[-2]
            cor_list = cor_list[-5:]
        #print(cor_list)
        
        
        m = m%len(str_list) #Make sure m index is within the range of the list
        received_strs = str_list[m].split(";") #Split the string into two strings for different lines on LCD
        
        #Create str1 and str2 for the two lines on the LCD
        str1 = received_strs[0]
        if len(received_strs) >= 2:
            str2 = received_strs[1]
        n = max(len(str1), len(str2))
        if n > 16:
            str1 = ljust(str1, n)
            str1 = str1 + " "*space + str1
            if len(received_strs) >= 2:
                str2 = ljust(str2, n)
                str2 = str2 + " "*space + str2
        
        #Scrolling effect
        display_str1 = str1[x_offset:x_offset+16] if n > 16 else str1
        display_str2 = str2[x_offset:x_offset+16] if n > 16 else str2
        #print(display_str1)
        #print(display_str2)
        
        #Display the strings on the LCD
        lcd.clear()
        lcd.move_to(0,0)
        lcd.putstr(display_str1)
        lcd.move_to(0,1)
        lcd.putstr(display_str2)

        #print("display")
        
        #Scrolling effect for the strings
        if n > 16:
            x_offset = (x_offset + 1)%(n-1+space)

        time.sleep(0.5)
    
    #Change m index for different reccomendation to display
    if len(str_list) != 0:
        m = (m + 1)%len(str_list)
