CC=bcm2708hardfp-gcc
CARD=21
NAME=carbou_das
CFLAGS=-W -Wall -Wextra -Wfatal-errors -O2 -static
APP=helloworld

all: $(APP)
	scp -P 622$(CARD) $^ pi@peri:$(NAME)

clean:
	rm $(APP)
