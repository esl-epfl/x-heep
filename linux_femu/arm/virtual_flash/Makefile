all: virtual_flash

virtual_flash: virtual_flash.cpp OverlayControl.c OverlayControl.h
	g++ -O0 -g -Wall virtual_flash.cpp OverlayControl.c -o virtual_flash -lpthread -lm -lcma

clean:
	rm -f virtual_flash
