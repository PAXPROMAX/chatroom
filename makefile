src=$(wildcard ./*.cpp)
obj=$(patsubst ./%.cpp, ./%.o, $(src))
args= -Wall -g

ALL:main
clean:$(obj)
	-rm -rf $(obj)

main:main.o mevent.o
	g++ -o main main.o mevent.o

main.o:main.cpp
	g++ -c main.cpp

mevent.o:mevent.cpp
	g++ -c mevent.cpp

.PHONY:clean All