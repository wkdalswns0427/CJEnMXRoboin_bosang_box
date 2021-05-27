import board
import neopixel
import time
import numpy as np

pixels = neopixel.NeoPixel(board.D18, 10)


def led(led, color, brightness):
    led -= 1
    color -= 1
    RGB = [0,(brightness,0,0),(0,brightness,0),(0,0,brightness)]
    RGB_arr = np.array(([(0,RGB[1]),(0,RGB[2]),(0,RGB[3])], [(1,RGB[3]),(1,RGB[1]),(2,RGB[2])], [(3,RGB[2]),(2,RGB[3]),(3,RGB[1])]))
    if led < 3:
        led_in = RGB_arr[led,color][0]
        
    elif led >=3 and led <6:
        led = led - 3
        led_in = RGB_arr[led,color][0] + 4
        
    else:
        led = led - 6
        led_in = RGB_arr[led,color][0] + 8
        
    color_in = RGB_arr[led,color][1]
    pixels[led_in] = color_in
    print(led_in)

led(8,1,255)
led(8,2,255)

time.sleep(0.1)
pixels.fill((0,0,0))
