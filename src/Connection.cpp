#include"Connection.h"
#include<string.h>
#include"util.h"
void Connection::SetCloseCallback(std::function<void(const std::shared_ptr<Connection>)>const& fn) {
    close_cb_ = std::move(fn);
}
Connection::Connection(int fd, std::weak_ptr<EventLoop> loop):fd_(fd), wk_loop_(loop) {
    socket_ = std::make_unique<Socket>();
    socket_->SetFd(fd_);
    socket_->SetNonBlocking();
    socket_->SetReuse();
}
Connection::~Connection() {
}

int Connection::Start() {
    channel_ = std::make_shared<Channel>(fd_, wk_loop_);
    channel_->SetReadCallback(std::bind(&Connection::Read, shared_from_this()));
    channel_->SetWriteCallback(std::bind(&Connection::Send, shared_from_this()));
    auto conn = shared_from_this();
    channel_->SetHolder(conn);
    Reset();
    return 0;
}
void Connection::Reset() {
    state_ = Connected;
    bytes_have_send = 0;
    http_parse_.Clear();
    channel_->SetListenEvents(Channel::READ_EVENT | Channel::ONE_SHOT | Channel::ET);
    channel_->AddToPoll();
}
std::shared_ptr<EventLoop> Connection::GetLoop() {
    return wk_loop_.lock();
}
int Connection::GetFd() {
    return fd_;
}
void Connection::Process() {
    DEBUG("Process start\n");
    // 解析HTTP请求
    auto read_ret = http_parse_.Parse();
    if (read_ret == HttpParse::NO_REQUEST) {
        channel_->SetListenEvents(Channel::READ_EVENT | Channel::ONE_SHOT | Channel::ET);
        channel_->AddToPoll();
        return;
    }
    DEBUG("Process end\n");
    http_parse_.DoRequest();
    DEBUG("Send start\n");
    Send();
    DEBUG("Send end\n");
}
void Connection::Read() {
    char buf[1024];
    while (true) {
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_read = read(fd_, buf, sizeof(buf));
        DEBUG("got message :\n%s\n", buf);
        if (bytes_read > 0) {
            http_parse_.read_buf_.Append(buf, bytes_read);
        }
        else if (bytes_read == -1) {
            if (errno == EINTR) {
                DEBUG("continue reading\n");
                continue;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                DEBUG("read end\n");
                break;
            }
        }
        else if (bytes_read == 0) {
            DEBUG("read EOF, client fd %d disconnected\n", fd_);
            Close();
            return;
        }
        else {
            DEBUG("Other error on client fd %d\n", fd_);
            Close();
            return;
        }
    }
    if (state_ == Connected)Process();
}
void Connection::Send() {
    int bytes_send = 0;
    while (1) {
        bytes_send = write(fd_, http_parse_.send_buf_.c_str() + bytes_have_send, http_parse_.send_buf_.GetSize());
        if (bytes_send == -1) {
            if (errno == EINTR) {
                continue;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                channel_->SetListenEvents(Channel::WRITE_EVENT | Channel::ET | Channel::ONE_SHOT);
                channel_->AddToPoll();
                DEBUG("Set_write_Events\n");
                return;
            }
            else {
                DEBUG("error\n");
                break;
            }
        }
        else if (bytes_send == 0) {
            DEBUG("client fd %d disconnected\n", fd_);
            Close(); return;
        }
        else {
            bytes_have_send += bytes_send;
            if (bytes_have_send >= http_parse_.send_buf_.GetSize())break;
        }
    }
    if (http_parse_.Get("Connection") != "keep-alive") {
        Close();
    }
    else Reset();
}
void Connection::Destory() {
    channel_->DeleteInPoll();
    channel_ = nullptr;
}
void Connection::Close() {
    printf("fd:%d closed\n", fd_);
    if (state_ == State::Closed)return;
    state_ = State::Closed;
    close_cb_(shared_from_this());
}
