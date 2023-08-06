src=$(wildcard ./Source/*.cpp)
obj=$(patsubst ./Source/%.cpp, ./Build/%.o, $(src))
args= -Wall -g
Build= ./Build/
inc= -I ./Include/
Source= ./Source/
Include= ./Include/
-l= -l mysqlclient
ALL:main client
clean:$(obj)
	-rm -rf $(obj)

main:$(Build)main.o $(Build)mevent.o $(Build)dbutil.o $(Build)threadpool.o
	g++ -o $@ $^ $(-l)

client:$(Source)client.cpp
	g++ $(inc) -o client $(Source)client.cpp

$(Include)%.o:$(Source)%.cpp
	g++ $(inc) -o $@ $< $(-l)

.PHONY: clean