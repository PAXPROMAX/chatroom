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
    MYSQL* mysql;
    char table[256];
    
public:
    Dbutil(const char *unix_socket, unsigned long clientflag);
    ~Dbutil();
    bool user_login_verify(const char* name, const char* password);
    bool user_register(const char* name, const char* password);
};