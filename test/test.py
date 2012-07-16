import time
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BOARD)
GPIO.setup(16, GPIO.OUT)
GPIO.setup(18, GPIO.IN, pull_up_down=GPIO.PUD_UP)

# output test
print('OUTPUT test')
for i in range(10):
	time.sleep(0.1)
	GPIO.output(16, GPIO.HIGH)
	time.sleep(0.1)
	GPIO.output(16, GPIO.LOW)
	
# input test
print('INPUT test')
while 1:
	GPIO.output(16, GPIO.input(18))
	time.sleep(0.02)  # 20 ms
