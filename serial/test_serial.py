import serial

DEV = '/dev/tty.wchusbserialfd12410'
port = serial.Serial(DEV)
print('connected to', port.name)

while True:
    line = port.readline()
    print(line.decode('utf-8'), end='')
