#define _mevent_h

#ifndef _netset_h
    #include "netset.h"
#endif

#ifndef EVENT_SIZE 
    #define EVENT_SIZE 1024
#endif

#ifndef _SYS_EPOLL_H
    #include<sys/epoll.h>
#endif

#ifndef _UNISTD_H
    #include<unistd.h>
#endif

#ifndef _ARPA_INET_H
    #include<arpa/inet.h>
#endif

#ifndef _CTYPE_H
    #include<ctype.h>
#endif

#ifndef _STDIO_H
    #include<stdio.h>
#endif

#ifndef _FCNTL_H
    #include<fcntl.h>
#endif

#ifndef _STDLIB_H
    #include<stdlib.h>
#endif

#ifndef _STRING_H
    #include<string.h>
#endif

#ifndef _SIGNAL_H
    #include<signal.h>
#endif

struct event
{
	int fd;		//fd
	int num;	//use?
	char buf[BUF_MAXSIZE]; //buf
	void(*func)(struct event* ev, void* args);	//cb
	void* arg;	//arg
    int status;
};

extern struct event ev[EVENT_SIZE + 1];

void error_exit(const char* err);
void init_event(struct event* ev, int num);
void event_sig(int sig);
void event_read_cb(struct event* ev, void* args);
void event_write_cb(struct event* ev, void *args);
void event_listen_cb(struct event* ev, void* args);
void eventset(struct event* ev, int fd, void(*func)(struct event* ev, void* args),void* arg);
void eventdel(struct event* ev);
void init_sock_bind(int *epfd, int num);