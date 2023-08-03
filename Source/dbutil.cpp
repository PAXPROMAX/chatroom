#include<iostream>
#include<mysql/mysql.h>
#include<cstdio>
#include<sys/stat.h>
#include<sys/file.h>
#include<unistd.h>
#include<cstring>
#define SQLCONFIGLINE 6
using namespace std;

class Dbutil
{
private:
    MYSQL* mysql;
    const char* db;
    const char *host; 
    const char *user;
    const char *passwd; 
    unsigned int port;
    const char *unix_socket;
    unsigned long clientflag;
    const char* table;
    
public:
    Dbutil(const char *host, const char *user, const char *passwd, 
    const char *db, unsigned int port, const char *unix_socket, unsigned long clientflag, const char*table);
    ~Dbutil();
    bool user_login_verify(const char* name, const char* password);
    bool user_register(const char* name, const char* password);
};


Dbutil::Dbutil(const char *host, const char *user, const char *passwd, 
    const char *db, unsigned int port, const char *unix_socket, unsigned long clientflag, const char* table) : host(host), user(user), passwd(passwd),
    db(db), port(port), unix_socket(unix_socket), clientflag(clientflag), table(table)
{
    //mysql = mysql_real_connect(mysql, "localhost", "root", "cctv7355608", "test", 3306, NULL, 0);
    this->mysql = mysql_init(NULL);
    if(this->mysql == NULL)
    {
        printf("mysql init fail: ");
    }
    this->mysql = mysql_real_connect(mysql, host, user, passwd, db, port, unix_socket, clientflag);
    if(this->mysql == NULL)
    {
        printf("mysql connect fail: ");
    }
}

Dbutil::~Dbutil()
{
    if(this->mysql != nullptr)
    {
        mysql_close(this->mysql);//关闭连接
    }
}


bool Dbutil::user_login_verify(const char* name, const char* password)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int fields, rows;
    char buf[4096];
    sprintf(buf, "select * from %s where name = '%s' and password = '%s';", this->table, name, password);
    printf("%s\n", buf);
    if(mysql_query(mysql, buf) != 0)//执行查询语句
    { 
        printf("user login err: %s\n", buf);
        return false;
    }
    result = mysql_store_result(mysql);//获取结果集
    rows = mysql_num_rows(result);//获取结果集的行数
    if(rows == 1)
    {
        row = mysql_fetch_row(result);
        printf("user login success: %s, password: %s, access: %s\n", row[0], row[1], password);
        mysql_free_result(result);//释放结果集所占内存
        return true;
    }
    else if(rows != 0)
    {
        printf("something wrong in sql\n");
    }
    else
    {
        printf("user login fail: %s, %s\n", name, password);
    }
    /**/
    mysql_free_result(result);//释放结果集所占内存
   return false;
}

bool Dbutil::user_register(const char *name, const char *password)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int fields, rows;
    char buf[4096];
    sprintf(buf, "select * from %s where name = '%s' and password = '%s';", this->table, name, password);
    printf("%s\n", buf);
    if(mysql_query(mysql, buf) != 0)//执行查询语句
    { 
        printf("user create err: %s\n", buf);
        return false;
    }
    result = mysql_store_result(mysql);//获取结果集
    rows = mysql_num_rows(result);//获取结果集的行数
    if(rows == 1)
    {
        row = mysql_fetch_row(result);
        printf("user create success: %s, password: %s", name, password);
        mysql_free_result(result);//释放结果集所占内存
        return true;
    }
    else if(rows != 0)
    {
        printf("something wrong in sql\n");
    }
    else
    {
        printf("user create fail: %s, %s\n", name, password);
    }
    /**/
    mysql_free_result(result);//释放结果集所占内存
   return false;
}
int main(int argc, char *argv[])
{
    char buf[4096], *str[SQLCONFIGLINE], name[256], password[256];
    int fd, i, num;
    fd = open("./sqlconfig", O_RDONLY | O_NONBLOCK, 0666);
    if(fd == -1){
        printf("open file fail: ");
    }
    num = read(fd, buf, 4096);
    i = 0;
    str[0] = strtok(buf, "\n");
    for(i = 1; i < SQLCONFIGLINE; i++)
    {
        str[i] = strtok(NULL, "\n");
    }
    for(i = 0; i < SQLCONFIGLINE; i++)
    {
        strtok(str[i], "=");
        str[i] = strtok(NULL, "\0");
    }
    cin >> name;
    cin >> password;
    Dbutil* util = new Dbutil(str[0], str[1], str[2], str[3], atoi(str[4]), NULL, 0, str[5]);
    util->user_login_verify(name, password);
    delete util;
    close(fd);
    return 0;
}
