#include"TcpServer.h"
#include"ThreadPool.h"
#include"assert.h"
#include"EventLoop.h"
#include<unistd.h>
#include"util.h"
thread_local ThreadData thread_data_;
TcpServer::TcpServer(const char* ip, int port) {
    loop_ = std::make_shared<EventLoop>();
    loop_->Init();
    unsigned int size = std::thread::hardware_concurrency();
    thread_pool_ = std::make_unique<ThreadPool>(size);
    for (int i = 0; i < size; ++i) {
        std::shared_ptr<EventLoop>sub_loop = std::make_shared<EventLoop>();
        sub_loop->Init();
        sub_loops_.push_back(std::move(sub_loop));
    }
    socket_ = std::make_unique<Socket>();
    socket_->Create();
    socket_->SetReuse();
    socket_->Bind(ip, port);
    accept_channel_ = std::make_shared<Channel>(socket_->GetFd(), loop_);

    for (int i = 0; i < sub_loops_.size(); ++i) {
        std::function<void()>sub_loop = std::bind(&EventLoop::Loop, sub_loops_[i]);
        thread_pool_->add(std::move(sub_loop));
    }
}
TcpServer::~TcpServer() {

}
void TcpServer::start() {
    socket_->Listen();
    accept_channel_->SetListenEvents(Channel::READ_EVENT);
    accept_channel_->SetReadCallback(std::bind(&TcpServer::NewConnection, weak_from_this()));
    accept_channel_->AddToPoll();
    loop_->Loop();
}
int TcpServer::NewConnection(std::weak_ptr<TcpServer>& wk_tcpserver) {
    auto tcpserver = wk_tcpserver.lock();
    if (tcpserver == nullptr) {
        assert(0);
        return -1;
    }
    int client_fd = -1;
    tcpserver->socket_->Accept(client_fd);
    if (tcpserver->connections_.size() > 1000) {
        printf("Connection more than 1000,connection closed\n");
        close(client_fd);
        return -1;
    }
    if (client_fd == -1)return -1;
    uint64_t random = client_fd % tcpserver->sub_loops_.size();
    printf("new fd: %d\n", client_fd);
    std::shared_ptr<Connection> conn = std::make_shared<Connection>(client_fd, tcpserver->sub_loops_[random]);
    conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, wk_tcpserver, std::placeholders::_1));
    assert(tcpserver->connections_.find(client_fd) == tcpserver->connections_.end());
    tcpserver->connections_[client_fd] = conn;
    tcpserver->sub_loops_[random]->QueueInLoop(std::bind(&Connection::Start, conn));
    return 0;
}
int TcpServer::RemoveConnection(std::weak_ptr<TcpServer>& wk_tcpserver, const std::shared_ptr<Connection>& conn) {
    auto tcpserver = wk_tcpserver.lock();
    if (tcpserver == nullptr)return -1;
    tcpserver->loop_->QueueInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, tcpserver, conn));
}
int TcpServer::RemoveConnectionInLoop(std::shared_ptr<Connection>& conn) {
    int client_fd = conn->GetFd();
    auto it = connections_.find(client_fd);
    //assert(it != connections_.end());
    if (it == connections_.end())return -1;
    connections_.erase(client_fd);
    conn->GetLoop()->QueueInLoop(std::bind(&Connection::Destory, conn));
    //printf("TcpServer::DeleteConnection,%d\n", client_fd);
    //printf("Conn size:%d\n", connections_.size());
    return 0;
}
