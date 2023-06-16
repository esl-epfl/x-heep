import serial
import sys
import os
import matplotlib.pyplot as plt

argLen = len(sys.argv)
print("Total arguments passed:", argLen)

if( argLen < 2 ):
    print ("Usage:", sys.argv[0], "[/dev/ttyUSBx]")
else:
        serialPort = serial.Serial(port = sys.argv[1], baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

        serialString = ""                           # Used to hold data coming over UART
        

        while(1):

            # Wait until there is data waiting in the serial buffer
            if(serialPort.in_waiting > 0):

                # Read data out of the buffer until a carraige return / new line is found
                serialString = serialPort.readline()
                if (serialString == b'index,data\r\n'):
                    list = []
                elif (serialString == b'Batch done!\r\n'):
                    plt.plot(list)
                    plt.show()
                else:
                    list.append(int(serialString.split(b',')[1]))
        exit()