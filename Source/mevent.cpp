#include"../Include/mevent.h"

struct event ev[EVENT_SIZE + 1];
void error_exit(const char* err)
{
	perror(err);
	exit(-1);
}

void init_event(struct event* ev, int num)
{
	for(int i = 0; i < num; i++)
	{
		ev[i].fd = -1;
		ev[i].func = NULL;
		ev[i].arg = NULL;
		ev[i].num = 0;
	}
}


//for signal quit and interrupt
void event_sig(int sig)
{
	fprintf(stdout, "detected quit or interrupt signal, shutdown in 3 seconds\n");
	int i = 0;
	int* epfd = (int*)ev[EVENT_SIZE].arg;
	for(i = 0; i < EVENT_SIZE; i++)
	{
		if(ev[i].fd != -1)
		{
			epoll_ctl(*epfd, EPOLL_CTL_DEL, ev[i].fd, NULL);
			send(ev[i].fd, "server has been shutdown\n", sizeof("server has been shutdown\n"), 0);
			close(ev[i].fd);
		}
	}
	epoll_ctl(*epfd, EPOLL_CTL_DEL, ev[i].fd, NULL);
	close(ev[i].fd);
	close(*epfd);
	fprintf(stdout, "shutdown complete\n");
	exit(0);
}

void event_read_cb(struct event* rev, void* args)
{
	int i;
	rev->num = recv(rev->fd, rev->buf, BUF_MAXSIZE, 0);
	for(i = 0; i < rev->num; i++)
	{
		rev->buf[i] = toupper(rev->buf[i]);
	}
	//TODO: del send
	send(rev->fd, rev->buf, rev->num, 0);
	event_write_cb(rev, args);
	return;
}

void event_write_cb(struct event* wev, void *args)
{
	int i;
	for(i = 0; i < EVENT_SIZE; i++)
	{
		if(wev->fd == ev[i].fd || ev[i].fd == -1)
		{
			continue;
		}
		send(ev[i].fd, wev->buf, wev->num, 0);
	}
	return;
}

void event_listen_cb(struct event* lev, void* args)
{
	struct sockaddr_in clientaddr;
	struct epoll_event epev;
	unsigned int socklen;
	int fd, i, set;
	int *epfd;
	epfd = (int*)args;
	for(i = 0; i < EVENT_SIZE; i++)
	{
		if(ev[i].fd == -1)	break;
	}
	fd = accept(lev->fd, (struct sockaddr*)&clientaddr, &socklen);
	printf("new connect: %s\n", inet_ntoa(clientaddr.sin_addr));
	if(fd == -1)
	{
		error_exit("client accept fail: ");
	}
	set = fcntl(fd, F_GETFD);
	set |= O_NONBLOCK;
	if(fcntl(fd, F_SETFD, set) == -1)
	{
		error_exit("file control set fail: ");
	};


	eventset(&ev[i], fd, event_read_cb, (void*)&ev[i]);
	epev.events = EPOLLIN | EPOLLET;
	epev.data.ptr = (void*)&ev[i];
	epoll_ctl(*epfd, EPOLL_CTL_ADD, fd, &epev);
	return;
}

//set an event
void eventset(struct event* ev, int fd, void(*func)(struct event* ev, void* args),void* arg)
{
	ev->fd = fd;
	ev->func = func;
	ev->arg = arg;
	ev->num = strlen(ev->buf);
}

//del an event
void eventdel(struct event* ev)
{
	ev->fd = -1;
	ev->func = NULL;
	ev->arg = NULL;
	ev->num = 0;
}

void init_sock_bind(int *epfd, int num)
{
	struct sockaddr_in servaddr;
	struct epoll_event epev;
	int listenfd, set;

	//init sockaddr_in
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVPORT);



	//init epoll_event
	epev.data.ptr = (void*)&ev[EVENT_SIZE];
	epev.events = EPOLLIN;
    
	
	//init listenfd
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd == -1)	error_exit("socket create fail: ");

	//ET for listenfd

	set = fcntl(listenfd, F_GETFD);
	set |= O_NONBLOCK;
	if(fcntl(listenfd, F_SETFD, set) == -1)
	{
		error_exit("set file control fail: ");
	}

	//listenfd reuse port
	int opt = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) == -1)	error_exit("setsockopt error");

	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
	{
		error_exit("bind fail: ");
	}
	if(listen(listenfd, num))
	{
		error_exit("listen fail: ");
	}

	//init event[1024](listenfd event)
	eventset(&ev[EVENT_SIZE], listenfd, event_listen_cb, (void*)epfd);

	if(epoll_ctl(*epfd, EPOLL_CTL_ADD, listenfd, &epev) == -1)
	{
		error_exit("epoll_ctl set fail: ");
	}
}
