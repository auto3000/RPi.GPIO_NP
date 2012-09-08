import time
import RPi.GPIO as GPIO

def test_output():
    print('OUTPUT test')
    for i in range(10):
            time.sleep(0.1)
            GPIO.output(16, GPIO.HIGH)
            time.sleep(0.1)
            GPIO.output(16, GPIO.LOW)

def test_input():
    print('INPUT test (Ctrl-C to stop)')
    try:
        while 1:
            GPIO.output(16, GPIO.input(18))
            time.sleep(0.02)  # 20 ms
    except KeyboardInterrupt:
        return

def test_rising():
        print('Rising edge test (test not implemented)')

def test_falling():
        print('Falling edge test')
        GPIO.set_falling_event(18)
        time.sleep(5)
        if GPIO.event_detected(18):
            print('Event detected')
        else:
            print('Event not detected')

def test_high():
        print('High detect test')
        GPIO.set_high_event(18)
        time.sleep(5)
        if GPIO.event_detected(18):
            print('Event detected')
        else:
            print('Event not detected')

def test_low():
        print('Low detect test (test not implemented)')

def test_gpio_function():
        for chan in range(54):
                f = GPIO.gpio_function(chan)
                if f == GPIO.IN:
                        func = 'INPUT'
                elif f == GPIO.OUT:
                        func = 'OUTPUT'
                elif f == GPIO.ALT0:
                        func = 'ALT0'
                else:
                        func = 'UNKNOWN'
                print('chan=%s func=%s'%(chan,func))

def test_force():
        test_passed = False
        try:
                GPIO.setup(8, GPIO.IN)   # is ALT0 serial TXD by default
        except GPIO.InvalidChannelException:
                test_passed = True
        if test_passed:
                print('Test passed')
        else:
                print('Test failed')

def test_setup():
        GPIO.setmode(GPIO.BOARD)
        GPIO.setup(16, GPIO.OUT)
        GPIO.setup(18, GPIO.IN, pull_up_down=GPIO.PUD_UP)



# main program starts here
while 1:
        print('1 - Setup')
        print('O - Output')
        print('I - Input')
        print('R - Rising edge')
        print('F - Falling edge')
        print('H - High detect')
        print('L - Low detect')
        print('G - gpio_function')
        print('C - forCe setup gpio')
        print('X - eXit')
        command = input('Enter your choice: ').upper()

        if command.startswith('1'):
                test_setup()
        elif command.startswith('O'):
                test_output()
        elif command.startswith('I'):
                test_input()
        elif command.startswith('R'):
                test_rising()
        elif command.startswith('F'):
                test_falling()
        elif command.startswith('H'):
                test_high()
        elif command.startswith('L'):
                test_low()
        elif command.startswith('G'):
                test_gpio_function()
        elif command.startswith('C'):
                test_force()
        elif command.startswith('X'):
                break

GPIO.cleanup()
