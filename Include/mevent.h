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

#ifndef _threadpool_h
    #include "threadpool.h"
#endif

struct event
{
	int fd;		//文件描述符
	int num;	//buf中的字节数

    
	char buf[BUF_MAXSIZE]; //用于接收数据
	void(*func)(void* arg);	//回调函数, 设置为epoll_read_cb

	void* arg;	//其他额外参数, 设置为指向epoll文件描述符的地址
    int status; //是否登录, 1为登录, 0为离线
};

extern struct event ev[EVENT_SIZE + 1];

extern struct threadpool_t *thp;
void error_exit(const char* err);
void init_event(struct event* ev, int num);
void set_nonblock(int fd);
void *process(void *arg);
void event_sig(int sig);
void event_read_cb(void* arg);
void event_write_cb(void *arg);
void event_listen_cb(void* args);
void eventset(struct event* ev, int fd, void(*func)(void* arg),void* arg, int* epfd);
void eventdel(struct event* ev);
void init_sock_bind(int *epfd, int num);