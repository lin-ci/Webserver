#pragma once
#include<thread>
class ThreadData {
public:
    ThreadData():tid_(std::this_thread::get_id()) {}
    ~ThreadData() = default;
    bool IsInThisThread() {
        return std::this_thread::get_id() == tid_;
    }
    const std::thread::id tid_;
};