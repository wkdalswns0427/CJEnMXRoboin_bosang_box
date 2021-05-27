import RPi.GPIO as GPIO
import time

pin_IR = 17
pin_servo_open = 16
pin_servo_lock = 27
pin_switch = 20
servoMax = 12
servoMin = 3


GPIO.setmode(GPIO.BCM)
GPIO.setup(pin_IR,GPIO.IN)
GPIO.setwarnings(False)
GPIO.setup(pin_servo_open, GPIO.OUT)
GPIO.setup(pin_servo_lock, GPIO.OUT)
GPIO.setup(pin_switch, GPIO.IN, pull_up_down=GPIO.PUD_UP) 

servo_open = GPIO.PWM(pin_servo_open, 50)
servo_lock = GPIO.PWM(pin_servo_lock, 50)
servo_open.start(0)
servo_lock.start(0)

IR_cnt = 0
switch_cnt = 0
lock_switch = 0

def servo_open_pos(degree):
    if degree > 180:
        degree = 180
    
    duty = servoMin + (degree*(servoMax-servoMin)/180.0)
    print("Degree: {}, Duty: {}".format(degree, duty))
    servo_open.ChangeDutyCycle(duty)

def servo_lock_pos(degree):
    if degree > 180:
        degree = 180
    
    duty = servoMin + (degree*(servoMax-servoMin)/180.0)
    print("Degree: {}, Duty: {}".format(degree, duty))
    servo_lock.ChangeDutyCycle(duty)

    
#open_init
#servo
servo_lock_pos(0)
servo_open_pos(0)
time.sleep(1)


while True:

    #lock
    if lock_switch == 0:
        #IR detected
        if GPIO.input(pin_IR) == 1:
            IR_cnt += 1
        
        #servo
        if IR_cnt >= 10:
            print("lock", IR_cnt)

            servo_lock_pos(120)
            #time.sleep(1)
            
            servo_open_pos(0)
            time.sleep(1)

            lock_switch = 1
            IR_cnt = 0

    #open
    #print(switch_cnt)
    elif lock_switch == 1:
        if GPIO.input(pin_switch) == 1:
            switch_cnt += 1
        #servo
        if switch_cnt >= 5:
            print("open", switch_cnt)

            servo_lock_pos(0)
            time.sleep(0.5)

            servo_open_pos(120)
            time.sleep(0.5)

            servo_open_pos(0)
            time.sleep(0.5)

            lock_switch = 0
            switch_cnt = 0

    #servo_lock.stop()
    #servo_open.stop()

    #pedometer
    time.sleep(0.1)
    print(IR_cnt, switch_cnt, lock_switch)

GPIO.cleanup