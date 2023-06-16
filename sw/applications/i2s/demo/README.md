# I2S DEMO application
The main.c read from the microphone and dumps the read values.

This demo requires mcu_gen with `MEM_BANKS=16`
to record 2.52 seconds of audio.

## demo python script
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

The tranmisting of the data takes about 30sec so be patient...



## alternative - dump to file with minicom
```
sudo minicom --b 115200 -D /dev/ttyUSBx -C LOG_FILE
```
and anlyse wherever...
