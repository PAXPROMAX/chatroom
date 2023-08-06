#include"../Include/mevent.h"
#include"../Include/dbutil.h"
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
		ev[i].status = 0;
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
	memset(rev->buf, 0, sizeof(rev->buf));
	rev->num = recv(rev->fd, rev->buf, BUF_MAXSIZE, 0);
/*
	for(i = 0; i < rev->num; i++)
	{
		rev->buf[i] = toupper(rev->buf[i]);
	}
	//TODO: del send for itself can't get message
	send(rev->fd, rev->buf, rev->num, 0);
*/
	event_write_cb(rev, args);
	return;
}

void event_write_cb(struct event* wev, void *args)
{
	int i;
	if(wev->status == 1)
	{
		if(strcmp(wev->buf, "./exit") != 0)
		{
			for(i = 0; i < EVENT_SIZE; i++)
			{
				if(wev->fd == ev[i].fd || ev[i].status == 0)	//itself or offline can't get message
				{
					continue;
				}
				send(ev[i].fd, wev->buf, wev->num, 0);
			}
		}
		else
		{
			wev->status = 0;
		}
	}
	else
	{
		char* ctrl;
		char* name;
		char* password;
		ctrl = strtok(wev->buf, "=");
		name = strtok(NULL, "=");
		password = strtok(NULL, "\0");
		if(*ctrl == '1')
		{
			Dbutil* util = new Dbutil(NULL, 0);
			if(util->user_login_verify(name, password) == 1)
			{
				send(wev->fd, "login success", sizeof("login success"), 0);
				wev->status = 1;
			}
			else
			{
				send(wev->fd, "login fail", sizeof("login fail"), 0);
			}
			delete util;
		}

		//wait for test
		else if(*ctrl == '2')
		{
			Dbutil* util = new Dbutil(NULL, 0);
			if(util->user_register(name, password) == 1)
			{
				send(wev->fd, "register success", sizeof("register success"), 0);
			}
			else
			{
				send(wev->fd, "register fail", sizeof("register fail"), 0);
			}
			delete util;
		}
		else if(*ctrl == '0')
		{
			wev->status = 0;
			eventdel(wev);
		}
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
	close(ev->fd);
	ev->fd = -1;
	ev->func = NULL;
	ev->arg = NULL;
	ev->num = 0;
	ev->status = 0;
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
