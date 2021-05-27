from bluetooth import *

socket = BluetoothSocket( RFCOMM )
socket.connect(("4C:11:AE:D5:6F:AE", 1))
print("bluetooth connected!")

while True:
    data = socket.recv(1024)
    print("Received: %s" %data)
    if(data=="q"):
        print("Quit")
        break

socket.close()