all: fake_flash

fake_flash: fake_flash.cpp OverlayControl.c OverlayControl.h
	g++ -O0 -g -Wall fake_flash.cpp OverlayControl.c -o fake_flash -lpthread -lm -lcma

clean:
	rm -f fake_flash

