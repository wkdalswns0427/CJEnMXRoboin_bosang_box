
import time
import pigpio


pi = pigpio.pi() # Connect to local Pi.

servoPin = 27
try:
    while 1:
        pi.set_servo_pulsewidth(servoPin, 1800)
        time.sleep(1)
        break

except KeyboardInterrupt:
    # switch servo off
    pi.set_servo_pulsewidth(servoPin, 0)
    pi.stop()

