# chatroom
    Linux version 5.19.0-50-generic (buildd@lcy02-amd64-030) (x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0, 
    GNU ld (GNU Binutils for Ubuntu) 2.38) #50-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul 10 18:24:29 UTC 2023
    C/MYSQL/SOCKET/EPOLL/THREADPOOL

1.服务器支持端口复用, 但是在本地测试时可能会出现客户端占用端口或者服务器关闭但是客户端尝试重连的情况
出现以上情况, 请用终端将端口相关的进程关闭.

    另外服务器端在关闭时可能会因为sleep函数导致线程关闭较慢, 不要多次 ctrl + c 尝试中断, 按下一次即可, 
    如果按下多次 ctrl + c 请改用 ctrl + \ 中断并退出

2.MYSQL的相关配置写在sqlconfig里面


创建表的语句:
    DROP DATABASE IF EXISTS test;

    CREATE DATABASE test;

    USE test;

    DROP TABLE IF EXISTS users;

    CREATE TABLE users(
        id INT PRIMARY KEY AUTO_INCREMENT,
        name VARCHAR(255) NOT NULL UNIQUE,
        password VARCHAR(255) NOT NULL
    );

3.client客户端中预定义了 SERVIP--->服务端IPV4地址 默认 127.0.0.1
                      SERVPORT----->服务端的端口 默认 9690
    如果需要修改请在client.cpp中修改后重新编译


    第三方依赖库: mysqlclient
