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
		ev[i].status = 0;
		ev[i].username.clear();
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
	int i, ret;
	struct event* rev;
	char buf[BUF_MAXSIZE];
	rev = (struct event*)arg; 
	rev->buf.clear();
	ret = 0;
	memset(buf, 0, BUF_MAXSIZE);
	while(1)
	{
		ret = recv(rev->fd, buf, BUF_MAXSIZE, 0);	/*不断recv直到返回-1, 将数据全部装入rwv->buf中,  结束循环while(1)*/
		if(ret > 0)
		{
			rev->buf.append(buf);
			continue;
		}
		else if(ret == -1 && EAGAIN == errno)
		{
			break;
		}
		else if(ret == 0)
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
	std::string str = wev->username;
	str.append("\n\t");
	str.append(wev->buf);
	printf("%s\n", wev->buf.c_str());
	printf("ev[%d]:status:%d\n", wev->fd, wev->status);
	if(wev->status == 1)											/*wev所指向的客户端已经登录*/
	{
		printf("compare: %d\n", wev->buf.compare("./exit\0"));
		if(wev->buf.compare("./exit") == 0)					//如果收到./exit, 说明客户端退出登录
		{
			user_offline(wev);
		}
		else														/*收到不是./exit, 说明客户端正常发送数据*/
		{
			for(i = 0; i < EVENT_SIZE; i++)
			{
				if(wev->fd == ev[i].fd || ev[i].status == 0)		/*event未启用(socket未连接)或者未上线则不会发送消息*/
				{
					continue;
				}
				printf("ev[%d]---->ev[%d], %s----->%s\n", wev->fd, ev[i].fd, wev->username.c_str(), ev[i].username.c_str());
				str = wev->username;
				str.append("\n\t");
				str.append(wev->buf);
				printf("send buf: %s\n", str.c_str());
				while(1)
				{
					ret = send(ev[i].fd, str.c_str(), str.size(), 0);	/*将wev->buf中的所有数据发送,  结束循环while(1)*/
					printf("ret: %d\n", ret);
					if(ret > 0)
					{
						str = str.erase(0, ret);
						if(str.size() == 0)
						{
							break;
						}
						continue;
					}
					else if(ret == 0)
					{
						eventdel(&ev[i]);
						break;
					}
					else if(EAGAIN == errno && ret == -1)
					{
						break;
					}
					else	error_exit("read error: ");
				}
			}
			printf("ev[%d]: send complete\n", wev->fd);
		}
	}
	else									/*wev指向的客户端没有登录, 对发来的信息进行SQL分析*/
	{
		if(user_split_ctrl(wev) == false)
		{
			return;
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
<<<<<<< HEAD
	reset_epolloneshot(*epfd, lev);
	printf("exit\n");
=======
	reset_epolloneshot(*epfd, lev);				/*重新设置EPOLLONESHOT*/
>>>>>>> 21f18a90c001e6d58cc3c43afe2773e97deee417
	return;
}


void eventset(struct event* ev, int fd, void(*func)(void* arg),void* arg, int* epfd)
{
	struct epoll_event epev;
	ev->fd = fd;
	ev->func = func;
	ev->arg = arg;
<<<<<<< HEAD
	ev->num = strlen(ev->buf);
=======
>>>>>>> 21f18a90c001e6d58cc3c43afe2773e97deee417
	epev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epev.data.ptr = ev;
	epoll_ctl(*epfd, EPOLL_CTL_ADD, fd, &epev);
}


void eventdel(struct event* ev)
{
	epoll_ctl(*(int*)ev->arg, EPOLL_CTL_DEL, ev->fd, NULL);
	close(ev->fd);
	ev->fd = -1;
	ev->func = NULL;
	ev->arg = NULL;
	user_offline(ev);
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


bool user_split_ctrl(struct event* wev)
{
		int i;
		char* ctrl;
		char* name;
		char* password;
		char* operation;
		ctrl = strtok(wev->buf.data(), "=");
		name = strtok(NULL, "=");
		password = strtok(NULL, "\0");
		if(*ctrl == '0')	//收到的首字节为0, 说明客户端退出
		{
			printf("ev[%d] exit\n", wev->fd);
			eventdel(wev);
			return false;
		}

		Dbutil* util = new Dbutil(NULL, 0);
		if(*ctrl == '1')		//收到的首字节为1, 说明客户端请求登录
		{
			i = comfirm_online(name);
			if(util->user_login_verify(name, password) == 1 && i == EVENT_SIZE)
			{
				send(wev->fd, "login success", sizeof("login success"), 0);
				user_online(wev, name);
			}
			else
			{
				send(wev->fd, "login fail", sizeof("login fail"), 0);
			}
		}


		else if(*ctrl == '2')		//收到的首字节为2, 说明客户端请求注册用户
		{
			if(util->user_register(name, password) == 1)
			{
				send(wev->fd, "register success", sizeof("register success"), 0);
			}
			else
			{
				send(wev->fd, "register fail", sizeof("register fail"), 0);
			}
		}
		else if(*ctrl == '3')			//收到的首字节为3, 说明客户端请求删除用户
		{
			i = comfirm_online(name);
			if(util->user_delete(name, password) == 1 && i == EVENT_SIZE)
			{
				send(wev->fd, "delete success", sizeof("delete success"), 0);
			}
			else
			{
				send(wev->fd, "delete fail", sizeof("delete fail"), 0);
			}
		}
		delete util;
		return true;
}


void user_online(struct event* ev, const char* name)
{
	printf("user online: %s\n", ev->username.c_str());
	ev->status = 1;
	ev->username = name;
}

void user_offline(struct event* ev)
{
	printf("user offline: %s\n", ev->username.c_str());
	ev->status = 0;
	ev->username.clear();
}

int comfirm_online(const char* name)
{
	int i;
	for(i = 0; i < EVENT_SIZE; i++)
	{
		if(ev[i].fd != -1)
		{
			if(ev[i].username.compare(name) == 0)
			{
				return i;
			}
		}
	}
	return i;
}