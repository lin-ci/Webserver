#pragma once
#include<vector>
#include<memory>
#include<sys/epoll.h>
#include"Timer.h"
#include<map>
#include"Channel.h"
#define MAX_EVENTS 1024
class Poller {
public:
    Poller();
    ~Poller();

    int UpdateChannel(std::shared_ptr<Channel>&, int timeout);
    int DeleteChannel(std::shared_ptr<Channel>&);
    std::vector<std::shared_ptr<Channel> >Poll(long timeout = -1);
private:
    void UpdateTimerNode(std::shared_ptr<Channel>& ch, int timeout);
    void DeleteTimerNode(std::shared_ptr<Channel>& ch);

    const int TIME_OUT = 60;
    int epfd_;
    epoll_event events_[MAX_EVENTS];
    std::map<int, std::weak_ptr<Channel> >chs_;
    //std::mutex mutex_chs_;
    std::unique_ptr<TimerManager> timer_manager_;
};