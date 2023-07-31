
#include"mevent.h"




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
	while(1)
	{
		num = epoll_wait(epfd, events, EVENT_SIZE + 1, 0);
		for(i = 0; i < num; i++)
		{
			if(events[i].events & EPOLLIN)
			{
				e = (struct event*)events[i].data.ptr;
				e->func(e, e->arg);
			}
			if(events[i].events & EPOLLOUT)
			{
				// ???
			}
		}
	}
	return 0;
}
