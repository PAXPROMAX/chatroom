#include"../Include/mevent.h"


int main(int argc, char* argv[])
{
	struct epoll_event events[EVENT_SIZE + 1];
	struct event* e;
	int epfd, num, i;
	signal(SIGINT, event_sig);	//注册信号函数, 在退出前将所有资源释放
	//signal(SIGQUIT, event_sig);
	epfd = epoll_create(128);

	init_event(ev, EVENT_SIZE + 1);	//初始化struct event[MAX_EVENT + 1]的数据

	init_sock_bind(&epfd, 128);		//创建并初始化监听文件描述符, 并将文件描述符放入epoll中
	thp = threadpool_create(3,100,100);   /*创建线程池，池里最小3个线程，最大100，队列最大100*/
	while(1)
	{
		num = epoll_wait(epfd, events, EVENT_SIZE + 1, 0);
		for(i = 0; i < num; i++)
		{
			if(events[i].events & EPOLLIN)
			{
				e = (struct event*)events[i].data.ptr;
				//e->func((void*)e);
				threadpool_add(thp, process, (void*)e);
			}
		}
	}
	return 0;
}
