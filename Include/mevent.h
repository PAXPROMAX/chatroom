#define _mevent_h

#ifndef _netset_h
    #include "netset.h"
#endif

#ifndef EVENT_SIZE          //客户端连接最大数量
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

#ifndef _STRING
    #include<string>
#endif
struct event
{
	int fd;		/*文件描述符*/
	int num;	/*buf中的字节数*/

    
	std::string buf; /*用于接收数据*/
	void(*func)(void* arg);	/*回调函数, 设置为epoll_read_cb*/

	void* arg;	    /*其他额外参数, 设置为指向epoll文件描述符的地址*/
    int status;     /*是否登录, 1为登录, 0为离线*/
};

extern struct event ev[EVENT_SIZE + 1];

extern struct threadpool_t *thp;

/**
 * @brief 输出错误并退出程序
 * @param err     错误信息
 * @return void
 */
void error_exit(const char* err);

/**
 * @brief 初始化event
 * @param ev     event首地址
 * @param num    event的数量
 * @return void
 */
void init_event(struct event* ev, int num);

/**
 * @brief 将文件描述符设置为NONBLOCK
 * @param fd     文件描述符
 * @return void
 */
void set_nonblock(int fd);

/**
 * @brief 线程池中的工作线程执行的任务函数
 * @param arg     任务参数, 存放event用于实现回调, 处理业务
 * @return void*, 仅返回NULL
 */
void *process(void *arg);

/**
 * @brief 注册中断信号函数
 * @param sig     捕获的信号
 * @return void
 */
void event_sig(int sig);

/**
 * @brief event的读事件发生后的回调函数
 * @param arg     参数, 通常设置为指向自身地址
 * @return void
 */
void event_read_cb(void* arg);

/**
 * @brief 服务器对event读到的数据进行处理
 * @param arg     参数, 通常设置为指向自身地址
 * @return void
 */
void event_write(void *arg);

/**
 * @brief event的监听事件发生后的回调函数
 * @param arg     参数, 通常设置为指向自身地址
 * @return void
 */
void event_listen_cb(void* args);

/**
 * @brief 设置event的数据以及与文件描述符相关的epoll_event的参数
 * @param ev       要重新设置的event
 * @param fd       文件描述符, ev中的fd将设置和该参数相同
 * @param func    回调函数
 * @param arg      参数, 通常设置为epoll文件描述符
 * @param epfd     epoll文件描述符, 用于调用epoll_ctl
 * @return void
 */
void eventset(struct event* ev, int fd, void(*func)(void* arg),void* arg, int* epfd);

/**
 * @brief 删除ev内的数据, 并关闭 ev 指向的文件描述符
 * @param ev       要删除的event地址
 * @return void
 */
void eventdel(struct event* ev);

/**
 * @brief 将监听文件描述符设置到epoll上
 * @param epfd       epoll的文件描述符
 * @param num       单次监听的最大数量
 * @return void
 */
void init_sock_bind(int *epfd, int num);

/**
 * @brief 将客户端发送的数据进行与SQL相关的处理(注册, 登录, 删除)
 * @param wev       客户端对应的event
 * @return void
 */
bool user_split_ctrl(struct event* wev);

void reset_epolloneshot(int epfd, struct event* ev);