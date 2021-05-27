import RPi.GPIO as GPIO
import time

servoPin = 12
servoMax = 12
servoMin = 3

GPIO.setmode(GPIO.BCM)
GPIO.setup(servoPin, GPIO.OUT)

servo = GPIO.PWM(servoPin, 50)
servo.start(0)

def setServoPos(degree):
    if degree > 180:
        degree = 180
    
    duty = servoMin + (degree*(servoMax-servoMin)/180.0)
    print("Degree: {}, Duty: {}".format(degree, duty))
    servo.ChangeDutyCycle(duty)

while True:
    setServoPos(0)
    time.sleep(1)
    setServoPos(120)
    time.sleep(1)
    
GPIO.cleanup