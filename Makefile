threads = 1

all: test

run: *.cpp
	g++ main.cpp -o ./run -O2 -lm -march=native -Wall -Wextra

test: run
	./run $(threads)
