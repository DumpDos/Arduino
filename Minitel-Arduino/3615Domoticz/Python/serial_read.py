import os
import serial
import urllib

ser = serial.Serial('/dev/ttyUSB1',57600, timeout=1)

print ser 
while ser: 

	x = ser.readline()
	url='"{}"'.format(x)
	if  "json" in x:
		os.system('wget '+url)

ser.close()
