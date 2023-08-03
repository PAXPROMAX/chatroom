
#include<mysql/mysql.h>
#include<stdio.h>
int main(int argc, char *argv[])
{
    MYSQL* mysql; 
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int fields, rows, i, j;
    mysql = mysql_init(NULL);//初始化连接
    mysql = mysql_real_connect(mysql, "localhost", "root", "cctv7355608", "test", 3306, NULL, 0);//建立一个到mysql数据库的连接
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
    mysql_close(mysql);//关闭连接
    return 0;
}
