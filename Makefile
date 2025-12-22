OPTIONS=-O3 --std=c++17

all: main

debug: OPTIONS=-O0 -g
debug: all

main: src/main.cpp 
	g++ ${OPTIONS} -I libsais/include/ src/main.cpp -o smr

clean: 
	rm -rf build/
