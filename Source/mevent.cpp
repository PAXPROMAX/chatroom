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
	threadpool_destroy(thp);						/*销毁线程池*/

	/*关闭反应堆*/
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
	char buf[BUF_MAXSIZE];
	rev = (struct event*)arg; 
	rev->buf.clear();
	rev->num = 0;
	while(1)
	{
		rev->num = recv(rev->fd, buf, BUF_MAXSIZE, 0);	/*不断recv直到返回-1, 将数据全部装入rwv->buf中,  结束循环while(1)*/
		if(rev->num > 0)
		{
			rev->buf.append(buf);
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
	printf("recv: %s\n", rev->buf.c_str());
	event_write(arg);
	return;
}

void event_write(void *arg)
{
	struct event* wev;
	int i, *epfd, ret;
	wev = (struct event*)arg;
	wev->num = strlen(wev->buf.c_str());
	if(wev->status == 1)											/*wev所指向的客户端已经登录*/
	{
		if(strcmp(wev->buf.c_str(), "./exit") == 0)					//如果收到./exit, 说明客户端退出登录
		{
			wev->status = 0;
		}
		else														/*收到不是./exit, 说明客户端正常发送数据*/
		{
			for(i = 0; i < EVENT_SIZE; i++)
			{
				if(wev->fd == ev[i].fd || ev[i].status == 0)		/*event未启用(socket未连接)或者未上线则不会发送消息*/
				{
					continue;
				}
				std::string str = wev->buf;
				while(1)
				{
					ret = send(ev[i].fd, str.c_str(), wev->num, 0);	/*将wev->buf中的所有数据发送,  结束循环while(1)*/
					if(ret > 0)
					{
						str = str.erase(ret);
						wev->num -= ret;
						continue;
					}
					else if(ret == 0)
					{
						break;
					}
					else if(EAGAIN == errno && ret == -1)
					{
						break;
					}
					else	error_exit("read error: ");
				}
			}
		}
	}
	else									/*wev指向的客户端没有登录, 对发来的信息进行SQL分析*/
	{
		user_conection_sql(wev);
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
		fd = accept(lev->fd, (struct sockaddr*)&clientaddr, &socklen);	//不断执行accept直到返回-1为止, 结束循环while(1)
		if(fd > 0){
			for(i = 0; i < EVENT_SIZE; i++)		//寻找一个未使用的event
			{
				if(ev[i].fd == -1)	break;
			}
			if(i == EVENT_SIZE)			//如果i到达EVENT_SIZE, 说明event数组已满, 不能继续新增连接, 停止本次连接
			{
				close(fd);
				return;
			}
				printf("new connect: %s\n", inet_ntoa(clientaddr.sin_addr));		
				if(ev[i].fd == -1)

				/*对新的文件描述符设置边缘触发模式, 并装入空闲event中*/
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
	reset_epolloneshot(*epfd, lev);				/*重新设置EPOLLONESHOT*/
	printf("exit\n");
	return;
}


void eventset(struct event* ev, int fd, void(*func)(void* arg),void* arg, int* epfd)
{
	struct epoll_event epev;
	ev->fd = fd;
	ev->func = func;
	ev->arg = arg;
	ev->num = strlen(ev->buf.c_str());
	epev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epev.data.ptr = ev;
	epoll_ctl(*epfd, EPOLL_CTL_ADD, fd, &epev);
}


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


	set_nonblock(listenfd);					/*设置listenfd为NONBLOCK模式*/

	//设置端口复用
	int opt = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) == -1)	error_exit("setsockopt error");

	//继续初始化listenfd
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
	{
		error_exit("bind fail: ");
	}
	if(listen(listenfd, num))
	{
		error_exit("listen fail: ");
	}

	//给listenfd设置一个event, 并注册回调函数
	eventset(&ev[EVENT_SIZE], listenfd, event_listen_cb, (void*)epfd, epfd);
}


void user_conection_sql(struct event* wev)
{
		char* ctrl;
		char* name;
		char* password;
		char* operation;
		ctrl = strtok(wev->buf.data(), "=");
		name = strtok(NULL, "=");
		password = strtok(NULL, "\0");
		if(*ctrl == '1')		//收到的首字节为0, 说明客户端请求登录
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


		else if(*ctrl == '2')		//收到的首字节为0, 说明客户端请求注册用户
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
		else if(*ctrl == '3')			//收到的首字节为0, 说明客户端请求删除用户
		{
			Dbutil* util = new Dbutil(NULL, 0);
			if(util->user_delete(name, password) == 1)
			{
				send(wev->fd, "delete success", sizeof("delete success"), 0);
			}
			else
			{
				send(wev->fd, "delete fail", sizeof("delete fail"), 0);
			}
			delete util;
		}
		else if(*ctrl == '0')	//收到的首字节为0, 说明客户端退出
		{
			wev->status = 0;
			eventdel(wev);
			return;
		}
}