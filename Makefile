OPTIONS=-O3 --std=c++17

all: main gen_test kernelize

debug: OPTIONS=-O0 -g
debug: all

build/libsais.o: libsais/src/libsais.c
	gcc -O3 -c -I libsais/include/ ./libsais/src/libsais.c -o build/libsais.o

main: build/libsais.o src/main.cpp
	g++ ${OPTIONS} -I libsais/include/ src/main.cpp build/libsais.o -o build/smr

gen_test: src/gen_test.cpp
	g++ ${OPTIONS} src/gen_test.cpp -o build/gen_test

kernelize: build/libsais.o src/kernelize.cpp
	g++ ${OPTIONS} -I libsais/include/ src/kernelize.cpp build/libsais.o -o build/kernelize

clean:
	rm -rf ./build/*
