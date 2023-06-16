import serial
import sys
import os
import matplotlib.pyplot as plt

argLen = len(sys.argv)
print("Total arguments passed:", argLen)

start = False

if( argLen < 3 ):
    print ("Usage:", sys.argv[0], "[/dev/ttyUSBx]", "[num of plots]")
else:
        serialPort = serial.Serial(port = sys.argv[1], baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

        numPlots = int(sys.argv[2])

        serialString = ""                           # Used to hold data coming over UART
        
        count = 0

        while(1):

            # Wait until there is data waiting in the serial buffer
            if(serialPort.in_waiting > 0):
                mylist = []
                print(serialString)

                # Read data out of the buffer until a carraige return / new line is found
                serialString = serialPort.readline()
                if (b'index' in serialString):
                    start = True
                    mylist = []
                    print("Started recording")
                elif (start == True and b'Batch done' in serialString):
                    count = count + 1
                    plt.plot(mylist)
                    plt.show()
                    start = False
                    if (numPlots == count):
                        break
                elif (start == True):
                    mylist.append(int(serialString.split(b',')[1]))
        exit()
