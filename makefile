src=$(wildcard ./*.c)
obj=$(patsubst ./%.c, ./%.o, $(src))
args= -Wall -g

ALL:main
clean:$(obj)
	-rm -rf $(obj)

main:main.o mevent.o
	gcc -o main main.o mevent.o

main.o:main.c
	gcc -c main.c

mevent.o:mevent.c
	gcc -c mevent.c

.PHONY:clean All