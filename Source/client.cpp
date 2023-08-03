#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#define SERVIP "127.0.0.1"
#define SERVPORT 9690
#define BUFSIZE 4048

void error_exit(const char* err)
{
	perror(err);
	exit(-1);
}

void* pthread_write(void* arg)
{
    int fd = *(int*)arg;
    char buf[BUFSIZE];
    int num;
    while(1)
    {
        scanf("%s", buf);
        send(fd, buf, strlen(buf) + 1, 0);
    }
    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    int fd, i, num, set;
    struct sockaddr_in servaddr;
    pthread_t tid;
    const char* servip;
    char buf[BUFSIZE] = {0};
    servip = SERVIP;
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, servip, &servaddr.sin_addr.s_addr);
    servaddr.sin_port = htons(SERVPORT);
    fd = socket(AF_INET, SOCK_STREAM, 0);
	set = fcntl(fd, F_GETFD);
	set |= O_NONBLOCK;
	if(fcntl(fd, F_SETFD, set) == -1)
	{
		error_exit("set file control fail: ");
	}
    if(fd == -1) error_exit("socket create fail: ");
    if(connect(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) error_exit("connect fail: ");
    pthread_create(&tid, NULL, pthread_write, &fd);
    pthread_detach(tid);
    while (1)
    {
        num = recv(fd, buf, BUFSIZE, 0);

        if(num > 0) printf(">>>%s\n", buf);

        else if(num == 0)
        {
            printf("server has been shutdonw\n");
            close(fd);
            exit(0);
        }
        else error_exit("read fail: ");
    }    
    return 0;
}