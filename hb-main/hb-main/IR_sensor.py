import RPi.GPIO as GPIO
import time

photoPin = 17

GPIO.setmode(GPIO.BCM)
GPIO.setup(photoPin,GPIO.IN)

while True:
    print(GPIO.input(photoPin))
    time.sleep(0.1)

