
#include<mysql/mysql.h>
#include<iostream>
#include<sstream>
#include<cstdio>
#include<string>
using namespace std;
class Dbutil
{
private:
    MYSQL* mysql;
    const string db;
    const char *host; 
    const char *user;
    const char *passwd; 
    unsigned int port;
    const char *unix_socket;
    unsigned long clientflag;
    
public:
    Dbutil(const char *host, const char *user, const char *passwd, 
    const char *db, unsigned int port, const char *unix_socket, unsigned long clientflag);
    ~Dbutil();
    bool user_login_verify(string name, string password);
};


Dbutil::Dbutil(const char *host, const char *user, const char *passwd, 
    const char *db, unsigned int port, const char *unix_socket, unsigned long clientflag) : db(db)
{
    this->mysql = mysql_init(NULL);
    if(this->mysql == NULL)
    {
        printf("mysql connect fail: ");
    }
    this->mysql = mysql_real_connect(mysql, host, user, passwd, db, port, unix_socket, clientflag);
    if(this->mysql == NULL)
    {
        printf("mysql connect fail: ");
    }
    string a = "aaa";
}

Dbutil::~Dbutil()
{
    if(this->mysql != nullptr)
    {
        mysql_close(this->mysql);//关闭连接
    }
}


bool Dbutil::user_login_verify(string name, string password)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int fields, rows, i, j;
    stringstream ss;
    ss << "select * from "<< this->db << " where " << name << " = " << password;
    cout << ss.str();
    /*

    //mysql = mysql_real_connect(mysql, "localhost", "root", "cctv7355608", "test", 3306, NULL, 0);
    i = mysql_query(mysql, "select * from users;");//执行查询语句
    result = mysql_store_result(mysql);//获取结果集
    fields = mysql_num_fields(result);//获取查询的列数, 
    rows = mysql_num_rows(result);//获取结果集的行数
    printf("field: %u, row: %u\n", fields, rows);
    for(i = 0; i < rows; i++)
    {
        row = mysql_fetch_row(result);//不断获取下一行，然后循环输出
        for(j = 0; i < fields; i++)
        {
            printf("%s", row[i]);
        }
    }
    mysql_free_result(result);//释放结果集所占内存
    */
   return false;
}
int main(int argc, char *argv[])
{
    string name;
    string password;
    string db = "test";
    cin >> name;
    cin >> password;

    name.pop_back();
    password.pop_back();
    db.pop_back();
    Dbutil* util = new Dbutil("localhost", "root", "cctv7355608", "test", 3306, NULL, 0);
    util->user_login_verify(name, password);
    delete util;
    return 0;
}
