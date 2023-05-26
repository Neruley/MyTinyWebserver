#ifndef LST_TIMER
#define LST_TIMER
/*
 * 带头尾结点的升序双向链表管理定时器。
 * 为每个连接创建一个定时器，将其添加到链表中，并按照超时时间升序排列。
 * 执行定时任务时，将到期的定时器从链表中删除。
 * 添加定时器的事件复杂度是O(n), 删除定时器的事件复杂度是O(1)。
 */

#include <time.h>
#include <netinet/in.h>
#include "log.h"

class util_timer;
struct client_data
{
    sockaddr_in address;
    int sockfd;
    util_timer *timer;
};

class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}
public:
    time_t expire;
    void (*cb_func)(client_data *);
    client_data *user_data;
    util_timer *prev;
    util_timer *next;
};

class sort_timer_lst
{
private:
    util_timer *head;
    util_timer *tail;

    void add_timer(util_timer *timer, util_timer *lst_head)
    {
        util_timer *prev = lst_head;
        util_timer *tmp = prev->next;
        while (tmp)
        {
            if (timer->expire < tmp->expire)
            {
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = prev;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }
        if (!tmp)
        {
            prev->next = timer;
            timer->prev = prev;
            timer->next = NULL;
            tail = timer;
        }
    }
public:
    sort_timer_lst() : head(NULL), tail(NULL) {}
    ~sort_timer_lst()
    {
        util_timer *tmp = head;
        while (tmp)
        {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }
    void add_timer(util_timer *timer){
        if (!timer) return ;
        if (!head){
            head = tail = timer;
            return ;
        }
        if (timer->expire < head->expire){
            timer->next = head;
            head->prev = timer;
            head = timer;
            return ;
        }
        add_timer(timer, head);
    }
    void adjust_timer(util_timer *timer){
        if (!timer) return ;
        util_timer *tmp = timer->next;
        if (!tmp || (timer->expire < tmp->expire)) return ;
        if (timer == head)
        {
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer(timer, head);
        }
        else
        {
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer, timer->next);
        }
    }
    void del_timer(util_timer *timer)
    {
        if (!timer) return;
        if ((timer == head) && (timer == tail))
        {
            delete timer;
            head = NULL;
            tail = NULL;
            return;
        }
        if (timer == head)
        {
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }
        if (timer == tail)
        {
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
            return;
        }
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    } 
    void tick()
    {
        if (!head) return;
        //printf( "timer tick\n" );
        LOG_INFO("%s", "timer tick");
        Log::get_instance()->flush();
        time_t cur = time(NULL);
        util_timer *tmp = head;
        while (tmp)
        {
            if (cur < tmp->expire)
            {
                break;
            }
            tmp->cb_func(tmp->user_data);
            head = tmp->next;
            if (head)
            {
                head->prev = NULL;
            }
            delete tmp;
            tmp = head;
        }
    }
};

#endif