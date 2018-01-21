[![Build Status](https://travis-ci.org/auto3000/RPi.GPIO_NP.svg?branch=master)](https://travis-ci.org/auto3000/RPi.GPIO_NP)

This package provides a class to control the GPIO on a NanoPi boards and is replacement of RPi.GPIO well known on Raspberry Pi platform.

Note that this module is unsuitable for real-time or timing critical applications.  This is because you can not predict when Python will be busy garbage collecting.  It also runs under the Linux kernel which is not suitable for real time applications - it is multitasking O/S and another process may be given priority over the CPU, causing jitter in your program.  If you are after true real-time performance and predictability, buy yourself an Arduino http://www.arduino.cc !

Original RPi notice:
Note that the current release does not support SPI, I2C, hardware PWM or serial functionality on the RPi yet.
This is planned for the near future - watch this space!  One-wire functionality is also planned.

Although hardware PWM is not available yet, software PWM is available to use on all channels.

For examples and documentation, visit http://sourceforge.net/p/raspberry-gpio-python/wiki/Home/


On NanoPi modules supported pin order is BOARD (not BCM!) - "GPIO.setmode(GPIO.BOARD)". 
Valid order is compatible with Physical pin numbering returned by WiringPi ("gpio readall"). For example: it means that gpio pin named GPIO.1 should be invoked by number 12 "GPIO.setup(12,GPIO.OUT)" - while WiringPi (wPi collumn) will call it as 1 ("gpio mode 1 out")! 
If you want to pull pin low use "GPIO.output(12,GPIO.LOW)" - while WiringPi will call it like ("gpio write 1 0")

Please follow this rules before you ask a question.

 +-----+-----+----------+------+---+-NanoPI M1+---+------+----------+-----+-----+
 |  H3 | wPi |   Name   | Mode | V | Physical | V | Mode | Name     | wPi |  H3 |
 +-----+-----+----------+------+---+----++----+---+------+----------+-----+-----+
 |     |     |     3.3v |      |   |  1 || 2  |   |      | 5v       |     |     |
 |  12 |   8 |    SDA.0 | ALT5 | 0 |  3 || 4  |   |      | 5v       |     |     |
 |  11 |   9 |    SCL.0 | ALT5 | 0 |  5 || 6  |   |      | 0v       |     |     |
 | 203 |   7 |   GPIO.7 |  OUT | 1 |  7 || 8  | 0 | ALT5 | TxD1     | 15  | 198 |
 |     |     |       0v |      |   |  9 || 10 | 0 | ALT5 | RxD1     | 16  | 199 |
 |   0 |   0 |     TxD2 | ALT5 | 0 | 11 || 12 | 0 | OUT  | GPIO.1   | 1   | 6   |
 |   2 |   2 |     RTS2 |  OFF | 0 | 13 || 14 |   |      | 0v       |     |     |
 |   3 |   3 |     CTS2 |  OFF | 0 | 15 || 16 | 0 | OFF  | RTS1     | 4   | 200 |
 |     |     |     3.3v |      |   | 17 || 18 | 0 | OFF  | CTS1     | 5   | 201 |
 |  64 |  12 |     MOSI | ALT4 | 0 | 19 || 20 |   |      | 0v       |     |     |
 |  65 |  13 |     MISO | ALT4 | 0 | 21 || 22 | 0 | ALT5 | RxD2     | 6   | 1   |
 |  66 |  14 |     SCLK | ALT4 | 0 | 23 || 24 | 0 | ALT4 | SPI0_CS  | 10  | 67  |
 |     |     |       0v |      |   | 25 || 26 | 0 | OFF  | SPDIFOUT | 11  | 17  |
 |  19 |  30 |    SDA.1 | ALT4 | 0 | 27 || 28 | 0 | ALT4 | SCL.1    | 31  | 18  |
 |  20 |  21 | PCM0DOUT |  OFF | 0 | 29 || 30 |   |      | 0v       |     |     |
 |  21 |  22 |  PCM0DIN |  OFF | 0 | 31 || 32 | 0 | OFF  | GPIO.26  | 26  | 7   |
 |   8 |  23 |  GPIO.23 |  OFF | 0 | 33 || 34 |   |      | 0v       |     |     |
 |  16 |  24 |     CTS3 |  OUT | 1 | 35 || 36 | 0 | ALT4 | TxD3     | 27  | 13  |
 |   9 |  25 |  GPIO.25 |  OFF | 0 | 37 || 38 | 0 | OFF  | RTS3     | 28  | 15  |
 |     |     |       0v |      |   | 39 || 40 | 0 | ALT4 | RxD3     | 29  | 14  |
 +-----+-----+----------+------+---+----++----+---+------+----------+-----+-----+
 |  H3 | wPi |   Name   | Mode | V | Physical | V | Mode | Name     | wPi |  H3 |
 +-----+-----+----------+------+---+-NanoPI M1+---+------+----------+-----+-----+

 +-----+----NanoPI M1 Debug UART---+----+
 |  H3 | wPi |   Name   | Mode | V | Ph |
 +-----+-----+----------+------+---+----+
 |     |     |       0v |      |   | 41 |
 |     |     |       5v |      |   | 42 |
 |   4 |  32 |     TxD0 | ALT5 | 0 | 43 |
 |   5 |  33 |     RxD0 | ALT5 | 0 | 44 |
 +-----+-----+----------+------+---+----+

In case of any questions ask them at FriendlyARM community forum: http://www.friendlyarm.com/Forum/viewforum.php?f=47
Probably someone will help you resolve problems.
