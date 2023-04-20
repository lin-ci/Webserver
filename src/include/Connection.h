#pragma once
#include"Common.h"
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include"Buffer.h"
#include"EventLoop.h"
#include"Socket.h"
#include"Channel.h"
#include<memory>
#include"HttpParse.h"
#include"Timer.h"
class Connection:public std::enable_shared_from_this<Connection> {
public:
    enum State {
        Invalid = 0,
        Connecting,
        Connected,
        Closed,
    };

public:
    Connection(int fd, std::weak_ptr<EventLoop> loop);
    ~Connection();
    int Start();
    void Close();
    void Destory();
    int GetFd();
    std::shared_ptr<EventLoop> GetLoop();
    void SetCloseCallback(std::function<void(const std::shared_ptr<Connection>)>const& fn);
    std::weak_ptr<TimerNode> GetTimerNode() {
        return timer_node_;
    }
    void SetTimerNode(std::shared_ptr<TimerNode>& node) {
        timer_node_ = node;
    }

private:
    void Process();

    void Read();
    void Send();
    void Reset();
private:
    int bytes_have_send;
    State state_;

    int fd_;
    std::unique_ptr<Socket> socket_;
    std::shared_ptr<Channel> channel_;
    std::weak_ptr<EventLoop> wk_loop_;

    std::function<void(const std::shared_ptr<Connection>)> close_cb_;
    HttpParse http_parse_;
    //std::mutex mutex_;
    std::weak_ptr<TimerNode> timer_node_;
};