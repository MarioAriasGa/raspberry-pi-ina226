.PHONY: clean

all:	ina226

ina226: ina226.c ina226.h
	gcc -g -Wall -Wextra -pedantic -std=c11 -D_DEFAULT_SOURCE -o ina226 ina226.c -lwiringPi -lm

clean:
	rm -rf ina226
