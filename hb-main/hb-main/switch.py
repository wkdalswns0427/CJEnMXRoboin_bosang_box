import RPi.GPIO as GPIO
import time

switch = 21

GPIO.setmode(GPIO.BCM)
GPIO.setup(switch, GPIO.IN, pull_up_down=GPIO.PUD_UP) 
	

while True:
    print(GPIO.input(switch))
    time.sleep(0.1)

