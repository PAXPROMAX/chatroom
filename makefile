src=$(wildcard ./Source/*.cpp)
obj=$(patsubst ./Source/%.cpp, ./Build/%.o, $(src))
args= -Wall -g
Build= ./Build/
Include= -I ./Include/
Source= ./Source/
-l= -l mysqlclient
ALL:main
clean:$(obj)
	-rm -rf $(obj)

main:$(Build)main.o $(Build)mevent.o $(Build)dbutil.o
	g++ -o main $(Build)main.o $(Build)mevent.o $(Build)dbutil.o $(-l)

$(Build)main.o:$(Source)main.cpp
	g++ -o $(Build)main.o -c $(Source)main.cpp

$(Build)mevent.o:$(Source)mevent.cpp
	g++ $(Include) -o $(Build)mevent.o -c $(Source)mevent.cpp 

$(Build)dbutil.o:$(Source)dbutil.cpp
	g++ $(Include) -o $(Build)dbutil.o -c $(Source)dbutil.cpp  -l mysqlclient
dbutil:$(Source)dbutil.cpp
	g++ -o dbutil $(Source)dbutil.cpp -l mysqlclient
.PHONY:clean All

userctrl:$(Source)userctrl.cpp
	g++ $(Include) -o userctrl $(Source)userctrl.cpp

client:$(Source)client.cpp
	g++ $(Include) -o client $(Source)client.cpp