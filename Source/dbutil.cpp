#include"../Include/dbutil.h"



/**
 * @brief 替换所有字符串
 * @param resource_str          源字符串
 * @param sub_str               需要替换的字符串
 * @param new_str               用于替换的字符串
 * @return  std::string 目标字符串
 */
std::string strreplace(std::string resource_str, std::string sub_str, std::string new_str)
{
    std::string dst_str = resource_str;
    std::string::size_type pos = 0;
    while((pos = dst_str.find(sub_str, pos)) != std::string::npos)   //替换所有指定子串
    {
        dst_str.replace(pos, sub_str.length(), new_str);
        pos += new_str.length();
    }
    return dst_str;
}


/**
 * @brief 重新设置关于SQL的转义字符
 * @param str           源字符串
 * @return string 目标字符串
 */
std::string reset_ESC(const char* str)
{
    std::string reset_str = str;
    reset_str = strreplace(reset_str, "'", "''");
    reset_str = strreplace(reset_str, "\\", "\\\\");
    return reset_str;
}




Dbutil::Dbutil(const char *unix_socket, unsigned long clientflag)
{
    char buf[4096], *str[SQLCONFIGLINE];
    int fd, i, num;
    fd = open("./sqlconfig", O_RDONLY | O_NONBLOCK, 0666);
    if(fd == -1){
        printf("open file fail: ");
    }
    num = read(fd, buf, 4096);
    i = 0;

    /*
        str[0] : host
        str[1] : user
        str[2] : password
        str[3] : database
        atoi(str[4]) : port
        str[5] : table
    */
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
    close(fd);

    strcpy(table, str[5]);
    //mysql = mysql_real_connect(mysql, "localhost", "root", "cctv7355608", "test", 3306, NULL, 0);
    this->mysql = mysql_init(NULL);
    if(this->mysql == NULL)
    {
        printf("mysql init fail: ");
    }
    this->mysql = mysql_real_connect(mysql, str[0], str[1], str[2], str[3], atoi(str[4]), unix_socket, clientflag);
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
    using namespace std;
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int fields, rows;
    char buf[4096];
    string reset_name;
    string reset_password;


    reset_name = reset_ESC(name);
    reset_password = reset_ESC(password);
    sprintf(buf, "select * from %s where name = '%s' and password = '%s';", this->table, reset_name.c_str(), reset_password.c_str());
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
        printf("user login success: id: %s, password: %s, access: %s\n", row[0], row[1], reset_password.c_str());
        mysql_free_result(result);//释放结果集所占内存
        return true;
    }
    else if(rows != 0)
    {
        printf("something wrong in sql\n");
    }
    else
    {
        printf("user login fail: name: %s, password: %s\n", reset_name.c_str(), reset_password.c_str());
    }
    mysql_free_result(result);//释放结果集所占内存
    return false;
}

bool Dbutil::user_register(const char *name, const char *password)
{
    using namespace std;
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int fields, rows;
    char buf[4096];
    string reset_name;
    string reset_password;


    reset_name = reset_ESC(name);
    reset_password = reset_ESC(password);
    sprintf(buf, "insert into %s(name, password) values('%s', '%s');", this->table, reset_name.c_str(), reset_password.c_str());
    if(mysql_query(mysql, buf) != 0)//执行查询语句
    { 
        printf("user create err: %s\n", buf);
        return false;
    }
    result = mysql_store_result(mysql);//获取结果集
    if(result == NULL)
    {
        printf("user create success: name: %s, password: %s", reset_name.c_str(), reset_password.c_str());
        mysql_free_result(result);//释放结果集所占内存
        return true;
    }
    else
    {
        printf("user create err: name: %s, password: %s\n -sql: %s\n", reset_name.c_str(), reset_password.c_str(), buf);
    }
    mysql_free_result(result);//释放结果集所占内存
    return false;
}