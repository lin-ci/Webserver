#pragma once
#include<map>
#include"Buffer.h"
#include<string>
class HttpParse {
public:
    HttpParse();
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    Buffer read_buf_;
    Buffer send_buf_;
public:
    int ParseLine();
    int ParseRequestLine(int star, int end);
    int ParseHeaders(int star, int end);
    int ParseContent(int star, int end);
    HTTP_CODE Parse();
    void DoRequest();
    bool AddStatusLine(const char* status, const char* title);
    bool AddHeaders(int content_len);
    bool AddContent(const char* text);
    void Clear() {
        checked_idx_ = 0;
        read_idx_ = 0;
        star_ = 0;
        end_ = 0;
        mp.clear();
        check_state_ = CHECK_STATE_REQUESTLINE;
        read_buf_.Clear();
        send_buf_.Clear();
    }
    std::string Get(const char* key) {
        return mp[key];
    }
    //void HttpParse::add_response(const char* format, ...);
    //void add_status_line(int status, const char* title);
private:
    int checked_idx_;
    int read_idx_;
    int star_;
    int end_;
    std::map<std::string, std::string> mp;
    CHECK_STATE check_state_;
};