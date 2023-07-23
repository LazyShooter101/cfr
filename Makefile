prog := KuhnPoker

all: build run

build:
	g++ .\$(prog).cpp -o $(prog) -O2

run:
	./$(prog).exe