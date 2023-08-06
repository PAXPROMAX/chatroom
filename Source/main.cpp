#include"../Include/mevent.h"


int main(int argc, char* argv[])
{
	struct epoll_event events[EVENT_SIZE + 1];
	struct event* e;
	int epfd, num, i;
	signal(SIGINT, event_sig);
	//signal(SIGQUIT, event_sig);
	epfd = epoll_create(128);
	init_event(ev, EVENT_SIZE + 1);
	//init listen socket and bind epoll
	init_sock_bind(&epfd, 128);
	thp = threadpool_create(3,100,100);   /*创建线程池，池里最小3个线程，最大100，队列最大100*/
	while(1)
	{
		num = epoll_wait(epfd, events, EVENT_SIZE + 1, 0);
		for(i = 0; i < num; i++)
		{
			if(events[i].events & EPOLLIN)
			{
				e = (struct event*)events[i].data.ptr;
				threadpool_add(thp, process, (void*)e);
			}
		}
	}
	return 0;
}
