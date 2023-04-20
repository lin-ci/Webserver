#pragma once
#include"Common.h"
#include<functional>
#include<memory>
#include"EventLoop.h"
#include"Connection.h"
class Channel:public std::enable_shared_from_this<Channel> {
public:
    Channel(int fd, std::weak_ptr<EventLoop> loop);
    ~Channel();

    int GetFd()const;
    short GetListenEvents()const;
    short GetReadyEvents()const;
    void HandleEvent()const;

    void SetListenEvents(short ev);
    void SetReadyEvents(short ev);
    void SetReadCallback(std::function<void()>const& callback);
    void SetWriteCallback(std::function<void()>const& callback);
    void AddToPoll(long timeout = 10);
    void DeleteInPoll();
    void SetHolder(std::shared_ptr<Connection>& holder);
    std::shared_ptr<Connection>GetHolder();
    static const short READ_EVENT;
    static const short WRITE_EVENT;
    static const short ET;
    static const short ONE_SHOT;
private:
    int fd_;
    std::weak_ptr<EventLoop> wk_loop_;
    short listen_events_;
    short ready_events_;
    //bool exist_;
    std::function<void()>read_callback_;
    std::function<void()>write_callback_;
    std::weak_ptr<Connection>holder_;
};