src=$(wildcard ./Source/*.cpp)
obj=$(patsubst ./Source/%.cpp, ./Build/%.o, $(src))
args= -Wall -g
Build= ./Build/
Include= ./Build/
Source= ./Source/
ALL:main
clean:$(obj)
	-rm -rf $(obj)

main:$(Build)main.o $(Build)mevent.o
	g++ -o main $(Build)main.o $(Build)mevent.o

$(Build)main.o:$(Source)main.cpp
	g++ -o $(Build)main.o -c $(Source)main.cpp

$(Build)mevent.o:$(Source)mevent.cpp
	g++ -o $(Build)mevent.o -c $(Source)mevent.cpp

dbutil:$(Source)dbutil.cpp
	g++ -o $(Build)dbutil.o -c $(Source)dbutil.cpp
.PHONY:clean All