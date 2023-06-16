import serial
import sys
import os
import matplotlib.pyplot as plt
import scipy
import sounddevice as sd
import numpy as np






def fft_plot(x, fs, title, y_label, filename):
    X = np.fft.fft(x)
    f = fs*np.fft.fftfreq(len(x))
    f_resolution = f[1] - f[0]
    print("Resolution is: ", f_resolution)
    i = np.argmax(X[0:len(x)//2])

    print(f"Peak at {f[i]} Hz")

    fig, axs = plt.subplots(2, tight_layout=True)
    fig.suptitle(f"{title}")

    t = np.arange(len(x))/fs
    axs[0].plot(t, x, linewidth=0.5)
    axs[0].set_title("time series")
    axs[0].set_xlim((0, (len(x)-1)/fs))
    axs[0].set_xlabel("t [s]")
    axs[0].set_ylabel(f"{y_label}")

    axs[1].plot(f, abs(X), ".", markersize=3)
    axs[1].set_xlim((0, 2e3))
    axs[1].set_title("FFT")
    axs[1].set_xlabel("f [Hz]")
    axs[1].set_ylabel("Magnitude")

    fig.savefig(f"../report/figures/{filename}.pdf")
    fig.show()


# name = file name without extension




CLK = int(20e6)
DIV = 8
fs = CLK // DIV // 64



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

                    # play

                    data = np.array(mylist, dtype=np.int16)  
                    sample = (data - data.mean())  / 2**12
                    sd.play(sample, fs)

                    # store as wav
                    scipy.io.wavfile.write(f"data/{filename}.wav", fs, sample)

                    # FFT
                    fft_plot(
                        x=data - data.mean(),
                        fs=fs,
                        title="Recording of a 440 Hz note\n(played from a smartphone)",
                        y_label="Magnitude\n(16bit signed)",
                        filename="fft_analysis"
                    )



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
