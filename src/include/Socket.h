#pragma once
#include<arpa/inet.h>
class Socket {
public:
    Socket();
    ~Socket();

    int GetFd()const;
    void SetFd(int fd);
    int Create();

    int Bind(const char* ip, uint16_t port)const;
    int Listen()const;
    int SetNonBlocking()const;
    int IsNonBlocking()const;

    int Accept(int& client_fd)const;
    int Connect(const char* ip, uint16_t port)const;
    size_t RecvBufSize()const;
    void SetReuse();
private:
    int fd_;
};