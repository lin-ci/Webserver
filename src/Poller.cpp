#include"Poller.h"
#include<cstring>
#include"Channel.h"
#include<unistd.h>
#include"Socket.h"
#include<assert.h>
#include"util.h"
#define MAX_EVENTS 1024
Poller::Poller() {
    epfd_ = epoll_create1(0);
    assert(epfd_ != -1);
    memset(events_, 0, sizeof(*events_) * MAX_EVENTS);
    timer_manager_ = std::make_unique<TimerManager>();
}
Poller::~Poller() {
    if (epfd_ != -1) {
        close(epfd_);
    }
}
int Poller::UpdateChannel(std::shared_ptr<Channel>& ch, int timeout) {
    //assert(epfd_ != -1);
    epoll_event ev{};
    if (ch->GetListenEvents() & Channel::READ_EVENT)ev.events |= EPOLLIN;
    if (ch->GetListenEvents() & Channel::WRITE_EVENT)ev.events |= EPOLLOUT;
    if (ch->GetListenEvents() & Channel::ET)ev.events |= EPOLLET;
    if (ch->GetListenEvents() & Channel::ONE_SHOT)ev.events |= EPOLLONESHOT;
    ev.data.fd = ch->GetFd();
    bool channel_exist = 0;//ONESHOT
    if (chs_.find(ch->GetFd()) != chs_.end()) channel_exist = 1;
    else chs_[ch->GetFd()] = ch;
    if (channel_exist == false) {
        //ch->SetExist();
        int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, ch->GetFd(), &ev);
        if (ret == -1) {
            perror("Channel add failed");
            return -1;
        }
        auto conn = ch->GetHolder();
        if (conn)timer_manager_->addTimer(conn, TIME_OUT);
    }
    else {
        int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, ch->GetFd(), &ev);
        if (ret == -1) {
            perror("Channel modify failed");
            return -1;
        }
        UpdateTimerNode(ch, TIME_OUT);
    }
    //printf("Update Channel fd: %d,ListenEvents:%d\n", ch->GetFd(), ch->GetListenEvents());
    return 0;
}
int Poller::DeleteChannel(std::shared_ptr<Channel>& ch) {
    int sockfd = ch->GetFd();
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, sockfd, NULL);
    DeleteTimerNode(ch);
    if (ret == -1) {
        perror("Channel delete failed");
        return -1;
    }
    chs_.erase(sockfd);
    //printf("Channel delete OK\n");
    return 0;
}

std::vector<std::shared_ptr<Channel> >Poller::Poll(long timeout) {
    std::vector<std::shared_ptr<Channel> >active_channels;
    int nfds = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
    if (nfds == -1) {
        perror("epoll wait error");
        return active_channels;
    }
    for (int i = 0; i < nfds; ++i) {
        int fd = events_[i].data.fd;
        int events = events_[i].events;
        std::weak_ptr<Channel> wkch = chs_[fd];
        auto ch = wkch.lock();
        if (ch) {
            short ready_event = 0;
            if (events & EPOLLIN)ready_event |= Channel::READ_EVENT;
            if (events & EPOLLOUT)ready_event |= Channel::WRITE_EVENT;
            ch->SetReadyEvents(ready_event);
            active_channels.emplace_back(ch);
            UpdateTimerNode(ch, TIME_OUT);
        }
        else {
            DEBUG("channel not exist,fd %d\n", fd);
        }
    }
    timer_manager_->handleExpiredEvent();
    return active_channels;
}
void Poller::UpdateTimerNode(std::shared_ptr<Channel>& ch, int timeout) {
    auto conn = ch->GetHolder();
    if (conn) {
        auto timernode = conn->GetTimerNode().lock();
        if (timernode) {
            timernode->setDeleted();
            timer_manager_->addTimer(ch->GetHolder(), timeout);
        }
    }
}
void Poller::DeleteTimerNode(std::shared_ptr<Channel>& ch) {
    auto conn = ch->GetHolder();
    if (conn) {
        auto timernode = conn->GetTimerNode().lock();
        if (timernode)timernode->setDeleted();
    }
}