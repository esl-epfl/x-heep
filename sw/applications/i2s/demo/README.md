# I2S DEMO application
The main.c read from the microphone and dumps the read values.


## a) Dump to file
```
sudo minicom --b 115200 -D /dev/ttyUSBx -C LOG_FILE
```


## b) python script 
First install
```
pip install pyserial matplotlib sounddevice  numpy
```

```
sudo apt-get install libasound-dev
```

Run
```
python i2s_test.py /dev/ttyUSBx 1
```
