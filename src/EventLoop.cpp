#include"EventLoop.h"
#include"util.h"
#include<unistd.h>
#include"sys/eventfd.h"
int CreateEventFd() {
    int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    return event_fd;
}
EventLoop::EventLoop() {
    poller_ = std::make_unique<Poller>();
    wakeup_fd_ = CreateEventFd();
}
void EventLoop::Init() {
    wakeup_channel_ = std::make_shared<Channel>(wakeup_fd_, weak_from_this());
    wakeup_channel_->SetReadCallback(std::bind(&EventLoop::handleRead, weak_from_this()));
    wakeup_channel_->SetListenEvents(Channel::READ_EVENT | Channel::ET);
    wakeup_channel_->AddToPoll();
}
void EventLoop::handleRead(const std::weak_ptr<EventLoop>& wk_eventloop) {
    auto eventloop = wk_eventloop.lock();
    if (eventloop != nullptr) {
        uint64_t one = 1;
        ssize_t n = read(eventloop->wakeup_fd_, (char*)&one, sizeof(one));
    }
}
void EventLoop::Wakeup() {
    DEBUG("wakeup\n");
    uint64_t one = 1;
    ssize_t n = write(wakeup_fd_, (char*)&one, sizeof(one));
    DEBUG("write :%d bytes\n", n);
}
EventLoop::~EventLoop() {
    DEBUG("EventLoop delete\n");
}
void EventLoop::Loop() {
    while (true) {
        printf("epoll_wait\n");
        auto active_channels = poller_->Poll(5000);
        for (std::shared_ptr<Channel>& ch : active_channels) {
            printf("channel handleEvent fd: %d\n", ch->GetFd());
            ch->HandleEvent();
        }
        DoPendingFunctors();
    }
}
void EventLoop::DoPendingFunctors() {
    std::queue<std::function<void()>> tasks;
    {
        std::lock_guard<std::mutex>lock(mutex_);
        std::swap(tasks, pendingFunctors_);
    }
    while (tasks.size()) {
        auto task = tasks.front(); tasks.pop();
        task();
    }
}

void EventLoop::RunInLoop(const std::function<void()>& fn) {
    if (thread_data_.IsInThisThread()) {
        fn();
    }
    else QueueInLoop(fn);
}
void EventLoop::QueueInLoop(const std::function<void()>& fn) {
    DEBUG("QueueInLoop\n");
    {
        std::lock_guard<std::mutex>lock(mutex_);
        pendingFunctors_.emplace(std::move(fn));
    }
    Wakeup();
}
void EventLoop::UpdateChannel(std::shared_ptr<Channel> ch, int timeout) {
    poller_->UpdateChannel(ch, timeout);
}
void EventLoop::DeleteChannel(std::shared_ptr<Channel> ch) {
    poller_->DeleteChannel(ch);
}