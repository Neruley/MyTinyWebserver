MyTinyWebserver
===============
* 用于学习理解Webserver的工作的原理
* 学习书籍：Linux高性能服务器编程   游双著
* 学习视频：牛客Linux高并发服务器开发
* 项目参考：https://github.com/qinguoyi/TinyWebServer/tree/raw_version

运行
------------
* 测试前确认已安装MySQL数据库

    ```C++
    // 建立yourdb库
    create database yourdb;
    
    // 创建user表
    USE yourdb;
    CREATE TABLE user(
        username char(50) NULL,
        passwd char(50) NULL
    )ENGINE=InnoDB;
    
    // 添加数据
    INSERT INTO user(username, passwd) VALUES('name', 'passwd');
    ```

* 修改main.c中的数据库初始化信息

    ```C++
    //数据库登录名 密码 库名  (根据实际修改)
    string user = "root";
    string passwd = "password";
    string databasename = "yourdb";
    ```

* 修改http_conn.cpp中的root路径

    ```C++
    // 修改为root文件夹所在路径
    const char *doc_root = "/home/ylone/MyTinyWebserver/root";
    ```

* 生成启动server

    ```C++
    make clean
    make
    ./server port
    ```

    最后在浏览器中输入运行主机的ip:port即可。
