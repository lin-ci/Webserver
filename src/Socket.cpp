#include"Socket.h"
#include<unistd.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<assert.h>
#include"util.h"
Socket::Socket():fd_(-1) {
}
Socket::~Socket() {
    printf("Socket closed fd:%d\n", fd_);
    if (fd_ != -1) {
        close(fd_);
        fd_ = -1;
    }
}
int Socket::GetFd()const {
    return fd_;
}
void Socket::SetFd(int fd) {
    assert(fd_ == -1);
    fd_ = fd;
}
int Socket::Create() {
    assert(fd_ == -1);
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ == -1) {
        perror("Create socket failed");
        return -1;
    }
    return 0;
}

int Socket::SetNonBlocking()const {
    int flag = fcntl(fd_, F_GETFL);
    if (flag == -1) {
        perror("Get Socket file flag error\n");
        return -1;
    }
    if (fcntl(fd_, F_SETFL, flag | O_NONBLOCK) == -1) {
        perror("Socket set non-blockong failed");
        return -1;
    }
    return 0;
}
int Socket::IsNonBlocking() const {
    int ret = fcntl(fd_, F_GETFL);
    if (ret == -1) {
        perror("Get Socket file flag error\n");
        return -1;
    }
    return (ret & O_NONBLOCK) != 0;
}
int Socket::Bind(const char* ip, uint16_t port)const {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (bind(fd_, (sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Failed to bind socket");
        return -1;
    }
    return 0;
}
int Socket::Listen()const {
    if (listen(fd_, SOMAXCONN) == -1) {
        perror("Failed to listen socket");
        return -1;
    }
    return 0;
}
int Socket::Accept(int& client_fd)const {
    client_fd = ::accept(fd_, NULL, NULL);
    if (client_fd == -1) {
        perror("Failed to accept socket");
        //assert(0);
        return -1;
    }
    return 0;
}
int Socket::Connect(const char* ip, uint16_t port)const {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (connect(fd_, (sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Failed to connect socket");
        return -1;
    }
    return 0;
}
size_t Socket::RecvBufSize() const {
    ssize_t size = -1;
    if (ioctl(fd_, FIONREAD, &size) == -1) {
        perror("Socket get recv buf size failed");
        return -1;
    }
    return size;
}
void Socket::SetReuse() {
    int reuse = 1;
    int ret = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (ret == -1) {
        perror("SetReuse error"); //assert(0);
    }
}