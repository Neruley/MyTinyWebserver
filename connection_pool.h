#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <list>
#include <mysql/mysql.h>
#include <string.h>
#include <iostream>
#include <error.h>
#include "locker.h"
using namespace std;

//数据库连接池
class connection_pool
{
private:
    unsigned int m_maxcoon;      //最大连接数
    unsigned int m_curcoon;      //当前已使用的连接数
    unsigned int m_freecoon;     //当前空闲的连接数
    locker lock;            
    sem reserve;
    list<MYSQL *> connlist;        //连接池
    string m_url;                  //主机地址
    int m_port;                 //数据库端口
    string m_user;                 //登录数据库账号
    string m_password;             //登录数据库密码
    string m_databasename;         //数据库名
public:
    connection_pool(){
        m_curcoon = 0;
        m_freecoon = 0;
    }
    ~connection_pool(){
        DestroyPool();
    }
    MYSQL *GetConnection()				 //获取数据库连接
    {
        MYSQL *con = NULL;
        if (connlist.empty()) return NULL;
        reserve.wait();
        lock.lock();
        con = connlist.front();
        connlist.pop_front();
        --m_freecoon;
        ++m_curcoon;

        lock.unlock();
        return con;
    }
	bool ReleaseConnection(MYSQL *conn) //释放连接
    {
        if (NULL == conn) return false;
        lock.lock();
        connlist.push_front(conn);
        ++m_freecoon;
        --m_curcoon;
        lock.unlock();
        reserve.post();
        return true;
    }
	int GetFreeConn()					 //当前空闲的连接数
    {
        return m_freecoon;
    }
	void DestroyPool()				 //销毁数据库连接池
    {
        lock.lock();
        if (connlist.size()){
            for (list<MYSQL *>::iterator it = connlist.begin(); it !=connlist.end(); ++it){
                MYSQL * con = *it;
                mysql_close(con);
            }
            m_curcoon = 0;
            m_freecoon = 0;
            connlist.clear();
        }
        lock.unlock();
    }
    //单例模式
    static connection_pool *getinstance(){
        static connection_pool connpool;
        return &connpool;
    }
    void init(int maxcoon, string url, int port, string user, string password, string databasename){
        m_maxcoon = maxcoon;
        m_url = url;
        m_port = port;
        m_user = user;
        m_password = password;
        m_databasename = databasename;
        
        lock.lock();
        for (int i=0; i<maxcoon; i++){
            MYSQL *con = NULL;
            con = mysql_init(con);
            if (con == NULL){
                // 报错需要加上endl清空缓冲区 不然不显示
                cout << "Error:" << mysql_error(con) << endl;
			    exit(1);
            }

            con = mysql_real_connect(con, url.c_str(), user.c_str(), password.c_str(), databasename.c_str(), port, NULL, 0);

            if (con == NULL){
                cout << "Error:" << mysql_error(con) << endl;
			    exit(1);
            }
            connlist.push_back(con);
            ++m_freecoon;
        }

        reserve = sem(m_freecoon);
        m_maxcoon = m_freecoon;
        lock.unlock();
    }
};

/*
Resource Acquisition Is Initialization，c++的一种资源管理机制
将资源调用封装为类的形式，在析构函数中释放资源，当对象生命结束时，系统会自动调用析构函数
通过connectionRAII类改变外部MYSQL指针使其指向连接池的连接，析构函数归还连接入池
*/
class connectionRAII
{
private:
    MYSQL *conRAII;
    connection_pool *poolRAII;
public:
    connectionRAII(MYSQL **con, connection_pool *connPool){
        *con = connPool->GetConnection();
        conRAII = *con;
        poolRAII = connPool;
    }
    ~connectionRAII(){
        poolRAII->ReleaseConnection(conRAII);
    }
};

#endif