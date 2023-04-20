#pragma once
#include"Common.h"
#include<memory>
#include<vector>
#include<map>
#include<functional>
#include"Connection.h"
#include<mutex>
#include"Thread.h"
#include"Socket.h"
#include"ThreadPool.h"
class TcpServer:public std::enable_shared_from_this<TcpServer> {
public:
    TcpServer(const char* ip, int port);
    ~TcpServer();
    void start();
private:
    static int NewConnection(std::weak_ptr<TcpServer>& wk_tcpserver);
    static int RemoveConnection(std::weak_ptr<TcpServer>& wk_tcpserver, const std::shared_ptr<Connection>& conn);
    int RemoveConnectionInLoop(std::shared_ptr<Connection>& conn);

private:
    std::shared_ptr<EventLoop>loop_;
    std::vector<std::shared_ptr<EventLoop>>sub_loops_;
    std::map<int, std::shared_ptr<Connection>> connections_;
    std::unique_ptr<ThreadPool>thread_pool_;
    std::unique_ptr<Socket> socket_;
    std::shared_ptr<Channel>accept_channel_;
};