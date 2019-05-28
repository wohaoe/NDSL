# threadpool
threadpool版的回射服务器



想写threadpool，所以就拿着个练练手



### 先说下各个文件的作用吧

myservere.cpp			服务器端主程序，用于接收connfd，并向线程池中添加任务

str_echo.cpp				服务器端回射代码写在这，很简单，直接收到就回射

condition.cpp			维护mutex和cond，对这两个参数进行封装

threadpool.cpp			线程池主程序，封装了各种对线程池的操作，并且负责调用回调函数。

my_unp.cpp				自己写的一个unp代码，里面封装了网络的初始化和常用函数

my_err.cpp				自己写的一个报错语句

myclient-easy.cpp		客户端主程序，功能：从键盘读，发送给服务器，接收，打印到屏幕





## 然后，说下思路吧

myserver.cpp开始，初始化网络（socket，bind，listen），然后for循环等待accept。每接到一个连接，就add一个task，把connfd作为参数。

画面转到 threadpool.cpp ，调用里面的threadpool_add_task()函数，此函数把一个task增加到poll里面，然后如果有空闲进程就唤醒空闲进程，如果没有的话就新建一个进程进行数据的处理。

接下来，无论是唤醒进程还是新建进程，都会转到执行thread_routine()函数，此函数可概括为三个功能

1. 如果此线程的任务执行完毕，则wait新任务到达，超时则销毁自身
2. 如果来了新任务（既poll->first != NULL)，则调用task里面的回调函数，并传task->args过去
3. 如果收到了线程池的销毁命令（quit），则对本线程自毁，如果线程池无线程则通吃线程池。

接下来转到回调函数，回调函数在myserver.cpp中，叫doit()，此函数负责调用str_echo()。

str_echo()函数则是正常的回射服务代码。



## 依赖关系

下面是各个文件的依赖关系，图画的有点水，见谅

!threadpool_return](http://pdsoe2moe.bkt.clouddn.com/threadpool_return.jpg)

结构可能规划的不对，欢迎大佬来指导。



其他的代码里面会有注释。



### 编译命令

哦，对了，编译命令如下

```bash
g++ myserver.cpp str_echo.cpp my_unp.cpp my_err.cpp condition.cpp threadpool.cpp -lpthread -o server

g++ myclient-easy.cpp my_unp.cpp my_err.cpp -o client
```



Ps.默认线程池最大值是3，可以在threadoiil_init函数的第二个参数那里改。



## 博客

嗯，有空可以照顾下我的博客

https://blog.csdn.net/qq_24889575/article/details/82692896

虽然里面也没啥。。。