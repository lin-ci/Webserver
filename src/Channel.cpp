#include"Channel.h"
#include"util.h"
//#include"EventLoop.h"
const short Channel::READ_EVENT = 1;
const short Channel::WRITE_EVENT = 2;
const short Channel::ET = 4;
const short Channel::ONE_SHOT = 8;
Channel::Channel(int fd, std::weak_ptr<EventLoop> loop):fd_(fd), wk_loop_(loop), listen_events_(0), ready_events_(0) {
}
Channel::~Channel() {
}
void Channel::SetHolder(std::shared_ptr<Connection>& holder) {
    holder_ = holder;
}
std::shared_ptr<Connection> Channel::GetHolder() {
    return holder_.lock();
}
int Channel::GetFd()const {
    return fd_;
}
short Channel::GetListenEvents()const {
    return listen_events_;
}
short Channel::GetReadyEvents()const {
    return ready_events_;
}
void Channel::HandleEvent()const {
    if (ready_events_ & READ_EVENT) {
        read_callback_();
    }
    if (ready_events_ & WRITE_EVENT) {
        write_callback_();
    }
}
void Channel::SetListenEvents(short ev) {
    listen_events_ = ev;
}
void Channel::AddToPoll(long timeout) {
    auto loop = wk_loop_.lock();
    if (loop == nullptr)return;
    auto ch = shared_from_this();
    loop->UpdateChannel(ch, timeout);
}
void Channel::DeleteInPoll() {
    auto loop = wk_loop_.lock();
    if (loop == nullptr)return;
    auto ch = shared_from_this();
    loop->DeleteChannel(ch);
}
void Channel::SetReadyEvents(short ev) {
    ready_events_ = ev;
}
void Channel::SetReadCallback(std::function<void()>const& callback) {
    read_callback_ = std::move(callback);
}

void Channel::SetWriteCallback(std::function<void()>const& callback) {
    write_callback_ = std::move(callback);
}