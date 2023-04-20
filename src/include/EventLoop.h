#pragma once
#include"Common.h"
#include<memory>
#include"Thread.h"
#include<queue>
#include<mutex>
#include"Poller.h"
#include<functional>
extern thread_local ThreadData thread_data_;
class EventLoop:public std::enable_shared_from_this<EventLoop> {
public:
    EventLoop();
    ~EventLoop();
    void Init();
    void Loop();
    void UpdateChannel(std::shared_ptr<Channel>, int timeout);
    void DeleteChannel(std::shared_ptr<Channel>);
    void RunInLoop(const std::function<void()>& fn);
    void QueueInLoop(const std::function<void()>& fn);
    void DoPendingFunctors();
    static void handleRead(const std::weak_ptr<EventLoop>& wk_eventloop);
    void Wakeup();
private:
    std::unique_ptr<Poller> poller_;
    //const int TIMEOUT = 10;
    std::queue<std::function<void()>>pendingFunctors_;
    std::mutex mutex_;
    int wakeup_fd_;
    std::shared_ptr<Channel>wakeup_channel_;
};