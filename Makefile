all: main

main: main.o FuncA.o
	g++ -o main main.o FuncA.o

main.o: main.cpp FuncA.h
	g++ -c main.cpp

FuncA.o: FuncA.cpp FuncA.h
	g++ -c FuncA.cpp

clean:
	rm -f *.o main

