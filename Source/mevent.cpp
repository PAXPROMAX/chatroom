#include"../Include/mevent.h"
#include"../Include/dbutil.h"
struct event ev[EVENT_SIZE + 1];

struct threadpool_t *thp;
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


void *process(void *arg)
{
	struct event* ev;
	ev = (struct event*)arg;
    printf("thread %u working on %d task\n",(unsigned int)pthread_self(),ev->fd);
	ev->func((void*)ev);
    printf("task %u is end\n", ev->fd);

    return NULL;
}

void reset_epolloneshot(int epfd, struct event* ev)
{
    epoll_event event;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    event.data.ptr = ev;
    epoll_ctl(epfd, EPOLL_CTL_MOD, ev->fd, &event);
}

void set_nonblock(int fd)
{
	int set;
	set = fcntl(fd, F_GETFL, 0);
	set |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, set) == -1)
	{
		error_exit("file control set fail: ");
	};
}

void event_sig(int sig)
{
	fprintf(stdout, "detected quit or interrupt signal, server shutdown\n");
	int i = 0;
	int* epfd = (int*)ev[EVENT_SIZE].arg;
	threadpool_destroy(thp);
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

void event_read_cb(void* arg)
{
	int i;
	struct event* rev;
	rev = (struct event*)arg; 
	memset(rev->buf, 0, sizeof(rev->buf));
	rev->num = 0;
	while(1)
	{
		rev->num = recv(rev->fd, &rev->buf[rev->num], BUF_MAXSIZE - rev->num, 0);
		if(rev->num > 0)
		{
			continue;
		}
		else if(rev->num == -1 && EAGAIN == errno)
		{
			break;
		}
		else if(rev->num == 0)
		{
			eventdel(rev);
			return;
		}
		else	error_exit("read error: ");
	}

	event_write(arg);
	return;
}

void event_write(void *arg)
{
	struct event* wev;
	int i, *epfd;
	wev = (struct event*)arg;
	wev->num = strlen(wev->buf);
	if(wev->status == 1)
	{
		if(strcmp(wev->buf, "./exit") != 0)
		{
			for(i = 0; i < EVENT_SIZE; i++)
			{
				if(wev->fd == ev[i].fd || ev[i].status == 0)
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
	epfd = (int*)wev->arg;
	reset_epolloneshot(*epfd, wev);
	return;
}

void event_listen_cb(void* arg)
{
	struct sockaddr_in clientaddr;
	struct event* lev;
	unsigned int socklen;
	int fd, i, set;
	int *epfd;
	lev = (struct event*)arg;
	epfd = (int*)lev->arg;
	
	while(1)
	{
		fd = accept(lev->fd, (struct sockaddr*)&clientaddr, &socklen);
		if(fd > 0){
			for(i = 0; i < EVENT_SIZE; i++)
			{
				if(ev[i].fd == -1)	break;
			}
			printf("new connect: %s\n", inet_ntoa(clientaddr.sin_addr));		
			if(ev[i].fd == -1)
			set_nonblock(fd);
			eventset(&ev[i], fd, event_read_cb, epfd, epfd);
		}
		else if(fd == -1 && errno == EAGAIN)
		{
			break;
		}
		else
		{
			error_exit("client accept fail: ");
		}
	}
	reset_epolloneshot(*epfd, lev);
	printf("exit\n");
	return;
}

//set an event
void eventset(struct event* ev, int fd, void(*func)(void* arg),void* arg, int* epfd)
{
	struct epoll_event epev;
	ev->fd = fd;
	ev->func = func;
	ev->arg = arg;
	ev->num = strlen(ev->buf);
	epev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epev.data.ptr = ev;
	epoll_ctl(*epfd, EPOLL_CTL_ADD, fd, &epev);
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
	int listenfd, set;

	// 初始化 sockaddr_in
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVPORT);


	// 初始化 listenfd
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd == -1)	error_exit("socket create fail: ");


	set_nonblock(listenfd);	/*设置listenfd为NONBLOCK模式*/

	//设置端口复用
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


	eventset(&ev[EVENT_SIZE], listenfd, event_listen_cb, (void*)epfd, epfd);
}
