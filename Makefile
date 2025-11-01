threads = 1
length = 1
frame = 0
fps = 15

all: test

run: *.cpp
	g++ main.cpp -o ./run -O2 -lm -march=native -Wall -Wextra

test: run
	./run $(threads) $(frame) $(fps)

video: run
	bash .video.sh $(threads) $(length) $(fps)
