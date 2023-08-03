#ifndef SERVPORT
    #define SERVPORT 9690
#endif

#ifndef BUF_MAXSIZE
    #define BUF_MAXSIZE 4096
#endif

#ifndef EVENT_SIZE 
    #define EVENT_SIZE 1024
#endif

#ifndef _sys_epoll_h_
    #include<sys/epoll.h>
#endif

#ifndef _unistd_h_
    #include<unistd.h>
#endif

#ifndef _arpa_inet_h_
    #include<arpa/inet.h>
#endif

#ifndef _ctype_h_
    #include<ctype.h>
#endif

#ifndef _stdio_h_
    #include<stdio.h>
#endif

#ifndef _fcntl_h_
    #include<fcntl.h>
#endif

#ifndef _stdlib_h_
    #include<stdlib.h>
#endif

#ifndef _string_h_
    #include<string.h>
#endif

#ifndef _signal_h_
    #include<signal.h>
#endif


struct event
{
	int fd;		//fd
	int num;	//use?
	char buf[BUF_MAXSIZE]; //buf
	void(*func)(struct event* ev, void* args);	//cb
	void* arg;	//arg
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
