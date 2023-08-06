#define _dbutil_h

#ifndef _GLIBCXX_IOSTREAM
    #include<iostream>
#endif

#ifndef _mysql_h
    #include<mysql/mysql.h>
#endif

#ifndef _STDIO_H
    #include<stdio.h>
#endif

#ifndef _SYS_STAT_H
    #include<sys/stat.h>
#endif

#ifndef _SYS_FILE_H
    #include<sys/file.h>
#endif

#ifndef _UNISTD_H
    #include<unistd.h>
#endif

#ifndef _STRING_H
    #include<string.h>
#endif

#ifndef _GLIBCXX_ALGORITHM
    #include<algorithm>
#endif

#ifndef SQLCONFIGLINE
    #define SQLCONFIGLINE 6
#endif

class Dbutil
{
private:
    MYSQL* mysql;       /*初始化连接后的返回值, 用于执行SQL语句*/
    char table[256];    /*连接数据库的表名*/
    
public:
/**
 * @brief 读取sqlconfig并初始化连接MYSQL 和table
 * @param unix_socket     使用unix连接方式，unix_socket为null时，表明不使用socket或管道机制
 * @param clientflag      设置为0 即可
 * @return *Dbutil
 */
    Dbutil(const char *unix_socket, unsigned long clientflag);

/**
 * @brief 关闭MySQL连接
 * @return void
 */
    ~Dbutil();

/**
 * @brief 连接MySQL验证登录
 * @param name          登录名称
 * @param password      登录密码
 * @return bool 成功为1, 失败为0
 */
    bool user_login_verify(const char* name, const char* password);

/**
 * @brief 连接MySQL注册账号
 * @param name          注册名称
 * @param password      注册密码
 * @return bool 成功为1, 失败为0
 */
    bool user_register(const char* name, const char* password);
};