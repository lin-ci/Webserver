#pragma once
#include"Common.h"
#include<string>
#include<memory>
#include<mutex>
class Buffer {
public:
    Buffer() = default;
    ~Buffer() = default;

    const std::string& GetBuf();
    const char* c_str();
    void SetBuf(const char* buf);

    size_t GetSize();
    void Append(const char* str, int size);
    void Append(const std::string& s) {
        buf_.append(s);
    }
    void Clear();
    char& operator[](int id) {
        return buf_[id];
    }
    int Find(const char* s, int pos = 0);
    std::string GetContext(int l, int r);
    //int ReadFd(int fd);
private:
    std::string buf_;
    //std::mutex mutex_;
};