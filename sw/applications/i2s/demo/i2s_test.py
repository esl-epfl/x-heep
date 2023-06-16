import serial
import sys
import os
import matplotlib.pyplot as plt

argLen = len(sys.argv)
print("Total arguments passed:", argLen)

start = False

if( argLen < 3 ):
    print ("Usage:", sys.argv[0], "[/dev/ttyUSBx]", "[num of batches]")
else:
        serialPort = serial.Serial(port = sys.argv[1], baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

        numPlots = int(sys.argv[2])

        serialString = ""# Used to hold data coming over UART
        count = 0
        mylist = []

        while(1):

            # Wait until there is data waiting in the serial buffer
            if(serialPort.in_waiting > 0):

                # read a line of data form the serial port
                serialString = serialPort.readline()

                # wait for index to signal start of data
                if (b'index' in serialString):
                    # reset variables
                    start = True
                    mylist = []
                    print("Started recording")
                elif (start == True and b'Batch done' in serialString):
                    print("Done recording")
                    print(mylist)

                    # DO ANYTHING WITH THE DATA HERE
                    # e.g. FFT, etc.

                    # Plot the data 
                    plt.figure(count)
                    plt.plot(mylist)
                    plt.show()

                    # restart the process
                    start = False
                    count = count + 1
                    if (numPlots == count):
                        break
                elif (start == True):
                    val = int(serialString.split(b',')[1])
                    mylist.append(val)
                else:
                    print(serialString)
        exit()
