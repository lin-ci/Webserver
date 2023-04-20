#include"HttpParse.h"
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<iostream>
#include"util.h"
#include<unistd.h>
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file from this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";


std::string doc_root_ = "/home/linxd/Documents/vscode/resources";
//获取一行
HttpParse::HttpParse() {
    DEBUG("HttpParse Create OK\n");

    checked_idx_ = read_idx_ = star_ = end_ = 0;
    check_state_ = CHECK_STATE_REQUESTLINE;
}
int HttpParse::ParseLine() {
    //return checked_idx_
    //-1 error
    //0 no /r/n
    DEBUG("start:%d,end:%d", star_, end_);
    char temp;
    read_idx_ = read_buf_.GetSize();
    for (; checked_idx_ < read_idx_; ++checked_idx_) {
        temp = read_buf_[checked_idx_];
        if (temp == '\r') {
            if ((checked_idx_ + 1) == read_idx_) {
                return 0;
            }
            else if (read_buf_[checked_idx_ + 1] == '\n') {
                checked_idx_ += 2;
                return checked_idx_ - 3;
            }
            return -1;
        }
        else if (temp == '\n') {
            if ((checked_idx_ > 1) && (read_buf_[checked_idx_ - 1] == '\r')) {
                checked_idx_ += 1;
                return checked_idx_ - 3;
            }
            return -1;
        }
    }
    return 0;
}
int HttpParse::ParseRequestLine(int star, int end) {
    // GET /index.html HTTP/1.1
    int pos = read_buf_.Find(" ", star);
    if (pos == -1) return -1;
    mp["method"] = read_buf_.GetContext(star, pos - 1);
    star = pos + 1;
    pos = read_buf_.Find(" \t", star);
    if (pos == -1) return -1;
    std::string url = read_buf_.GetContext(star, pos - 1);
    auto pre = url.find('/');
    if (url.substr(0, pre) == "http:")pre += 7;
    pre = url.find_first_of('/', pre);
    url = url.substr(pre);
    mp["url"] = url;
    star = pos + 1;
    mp["version"] = read_buf_.GetContext(star, star + 7);
    if (star + 7 > end)return -1;
    return 0;
}

int HttpParse::ParseHeaders(int star, int end) {
    int pos = read_buf_.Find(":", star);
    if (pos == -1)return -1;
    mp[read_buf_.GetContext(star, pos - 1)] = read_buf_.GetContext(pos + 2, end);
    return 0;
}
int HttpParse::ParseContent(int star, int end) {
    int length = atoi(mp["Content-Length"].c_str());
    if (end - star == length)return 0;
    return -1;
}
HttpParse::HTTP_CODE HttpParse::Parse() {
    while (true) {
        // 获取一行数据
        end_ = ParseLine();
        if (end_ == -1)return BAD_REQUEST;
        if (end_ == 0)return NO_REQUEST;
        switch (check_state_) {
        case CHECK_STATE_REQUESTLINE: {
            int ret = ParseRequestLine(star_, end_);
            if (ret == -1) return BAD_REQUEST;
            check_state_ = CHECK_STATE_HEADER;
            star_ = end_ + 3;
            break;
        }
        case CHECK_STATE_HEADER: {
            if (end_ < star_) {
                if (atoi(mp["Content - Length"].c_str()) > 0) {
                    check_state_ = CHECK_STATE_CONTENT;
                    star_ = end_ + 3;
                    break;
                }
                else return GET_REQUEST;
            }
            int ret = ParseHeaders(star_, end_);
            star_ = end_ + 3;
            if (ret == -1)return BAD_REQUEST;
            break;
        }
        case CHECK_STATE_CONTENT: {
            if (ParseContent(star_, end_) == 0)return GET_REQUEST;
        }
        default: {
            return INTERNAL_ERROR;
        }
        }
    }
    return NO_REQUEST;
}
bool HttpParse::AddStatusLine(const char* status, const char* title) {
    send_buf_.Append("HTTP/1.1 ", 9);
    send_buf_.Append(status, strlen(status));
    send_buf_.Append(" ", 1);
    send_buf_.Append(title, strlen(title));
    send_buf_.Append("\r\n", 2);
}
bool HttpParse::AddHeaders(int content_len) {
    send_buf_.Append("Content-Length: ", 16);
    send_buf_.Append(std::to_string(content_len));
    send_buf_.Append("\r\n");
    send_buf_.Append(std::string("Content-Type:text/html\r\n"));
    if (mp["Connection"] == "keep-alive")
        send_buf_.Append(std::string("Connection: keep-alive\r\n"));
    else send_buf_.Append(std::string("Connection: close\r\n"));
    send_buf_.Append(std::string("\r\n"));
}
bool HttpParse::AddContent(const char* text) {
    send_buf_.Append(text, strlen(text));
}

void HttpParse::DoRequest() {
    //"/home/linxd/webserver/resources"
    if (mp["method"] == "GET") {
        std::string file_ = doc_root_ + mp["url"];
        const char* real_file_ = file_.c_str();
        DEBUG(" OK\n");
        // 获取m_real_file文件的相关的状态信息，-1失败，0成功
        struct stat file_stat_;
        // 目标文件的状态。通过它我们可以判断文件是否存在、是否为目录、是否可读，并获取文件大小等信息
        if (stat(real_file_, &file_stat_) < 0) {
            AddStatusLine("404", error_404_title);
            AddHeaders(strlen(error_404_form));
            AddContent(error_404_form);
            return;
        }

        // 判断访问权限
        if (!(file_stat_.st_mode & S_IROTH)) {
            AddStatusLine("403", error_403_title);
            AddHeaders(strlen(error_403_form));
            AddContent(error_403_form);
            return;
        }

        // 判断是否是目录
        if (S_ISDIR(file_stat_.st_mode)) {
            AddStatusLine("400", error_400_title);
            AddHeaders(strlen(error_400_form));
            AddContent(error_400_form);
            return;
        }

        // 以只读方式打开文件
        int fd = open(real_file_, O_RDONLY);
        AddStatusLine("200", ok_200_title);
        AddHeaders(file_stat_.st_size);
        char buf[1024] = {};
        int read_bytes = 0;
        int tot = 0;
        while (1) {
            read_bytes = read(fd, buf, sizeof(buf));
            if (read_bytes > 0)
                send_buf_.Append(buf, read_bytes);
            else if (read_bytes <= 0) {
                break;
            }
            tot += read_bytes;
            memset(buf, 0, sizeof(buf));
        }
        close(fd);
        return;
    }
}