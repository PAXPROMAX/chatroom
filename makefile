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

client:$(Build)client.o
	g++ $(inc) -o $@ $<

$(Build)%.o:$(Source)%.cpp
	g++ $(inc) -c -o $@ $< $(-l)

.PHONY: clean