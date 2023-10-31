#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#define SERVIP "39.101.66.152"
#define BUF_MAXSIZE 4096
#define SERVPORT 9690
char ctrl;
void error_exit(const char* err)
{
	perror(err);
	exit(-1);
}
void user_menu()
{
    printf("*****************\n");
    printf("** 1.login     **\n");
    printf("** 2.register  **\n");
    printf("** 3.delete    **\n");
    printf("** 0.exit      **\n");
    printf("*****************\n");
    return;
}


void set_ctrl_char(char* ctrl)
{
    user_menu();
    char flusher;
    scanf("%c", ctrl);
    while(scanf("%c", &flusher))
	{
		if(flusher == '\n')
		{
			flusher = 0;
			break;
		}
	}
}

void user_input(char* buf, int size)
{
    int i = 0;
    while(i < size && scanf("%c", &buf[i]))
	{
	    if(buf[i] == '\n')
	    {
	    	buf[i] = 0;
	    	break;
	    }
        i++;
	}
}

int user_operation(char* buf, int size, char* ctrl)
{
    char name[256], password[256];
    printf("input name: ");
    user_input(name, 256);
    if(strcmp(name, "./exit") == 0)
    {
        set_ctrl_char(ctrl);
        return 0;
    }
    printf("input password: ");
    user_input(password, 256);
    if(strcmp(password, "./exit") == 0)
    {
        set_ctrl_char(ctrl);
        return 0;
    }
    if(*ctrl == '2')    //register == '2', other operation don't need comfirm
    {
        char comfirm[256];
        printf("input same password: ");
        user_input(comfirm, 256);
        if(strcmp(comfirm, "./exit") == 0)
        {
            set_ctrl_char(ctrl);
            return -1;
        }
        if(strcmp(password, comfirm) != 0)
        {
            printf("different password, regiser fail\n");
            set_ctrl_char(ctrl);
            return -1;
        }
    }
    if(size > sizeof(name) + sizeof(password) + 2)
    {
        snprintf(buf, size, "%c=%s=%s", *ctrl, name, password);
    }
    else
    {
        printf("user ctrl fail: lenth of name and password is too long or buffer size is too small\n");
        exit(-1);
    }
    return 0;
}

void user_exit(int fd)
{
    send(fd, "0", sizeof("0"), 0);
    close(fd);
    printf("user exit\n");
    exit(0);
}


void* pthread_write(void* arg)
{
    int fd = *(int*)arg;
    char buf[BUF_MAXSIZE];
    int num, time, ret;
    while(1)
    {
        memset(buf, 0,sizeof(buf));
        if(ctrl == '0')
        {
            send(fd, "0=0=0", sizeof("0=0=0"), 0);
            user_exit(fd);
        }
        else if(ctrl != '9')
        {
            while(user_operation(buf, BUF_MAXSIZE, &ctrl) == -1)
            {
                
            }
            send(fd, buf, strlen(buf), 0);
            printf("send: %s\n", buf);
            time = 6;
            num = 0;
            printf("set complete\n");
            num = recv(fd, buf, BUF_MAXSIZE, 0);
            fprintf(stdout, "strcmp: %d, buf: %s\n",strcmp(buf, "login success"), buf);
            if(strcmp(buf, "login success") == 0)
            {
                ctrl = '9';
                printf("login success\n");
                continue;
            }
            else if(strcmp(buf, "login fail") == 0)
            {
                printf("login fail\n");
            }
            else if(strcmp(buf, "register success") == 0)
            {
                printf("register success\n");
                set_ctrl_char(&ctrl);
            }
            else if(strcmp(buf, "register fail") == 0)
            {
                printf("register fail\n");
            }
            else if(strcmp(buf, "delete success") == 0)
            {
                printf("delete success\n");
            }
            else if(strcmp(buf, "delete fail") == 0)
            {
                printf("delete fail\n");
            }
            set_ctrl_char(&ctrl);
        }
        else if(ctrl == '9')
        {
            user_input(buf, BUF_MAXSIZE);

            if(strcmp(buf, "./exit") == 0) 
            {
                ctrl = '8';
                send(fd, "./exit", strlen("./exit"), 0);
                printf("send: %s\n", buf);
                set_ctrl_char(&ctrl);
            }
            else
            {
                send(fd, buf, sizeof(buf), 0);
                printf("send: %s\n", buf);
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int fd, i, num, set;
    struct sockaddr_in servaddr;
    pthread_t tid;
    const char* servip;
    char buf[BUF_MAXSIZE] = {0};

    set_ctrl_char(&ctrl);

    if(ctrl == '0')
    {
        user_exit(fd);
    }

    servip = SERVIP;
    inet_pton(AF_INET, servip, &servaddr.sin_addr.s_addr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVPORT);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) error_exit("socket create fail: ");
    if(connect(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) error_exit("connect fail: ");

    pthread_create(&tid, NULL, pthread_write, &fd);
    pthread_detach(tid);

    while (1)
    {
        if(ctrl == '9')
        {
            memset(buf, 0, sizeof(buf));
            num = 0;
            do
            {
                num = recv(fd, buf, BUF_MAXSIZE, MSG_DONTWAIT);
                if(num > 0)
                {
                    printf(">>>%s\n", buf);
                    printf("num = %d\n", num);
                    printf("\n");
                    continue;
                }
            }
            while(0);
            if(num == 0)
            {
                printf("server has been shutdown\n");
                close(fd);
                exit(0);
            }
            else if(num == -1)
            {
                if(errno == EAGAIN) continue;
                else error_exit("read fail: ");
            }
        }
    }    
    return 0;
}