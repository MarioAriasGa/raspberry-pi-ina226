.PHONY: clean

ina226: ina226.c ina226.h
	gcc -g -Wall -Wextra -pedantic -std=c11 -o ina226 ina226.c -lwiringPi -lm

clean:
	rm ina226
