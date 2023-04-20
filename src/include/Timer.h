#pragma once
#include"Common.h"
#include"Connection.h"
#include<memory>
#include<mutex>
#include<queue>
class TimerNode {
public:
    TimerNode(std::shared_ptr<Connection> conn, int timeout);
    ~TimerNode();
    TimerNode(const TimerNode& tn);
    bool isValid();
    bool isDeleted();
    void setDeleted();
    size_t getExpTime();
    void update(int timeout);
    std::shared_ptr<Connection> GetConnection() {
        return conn_;
    }
private:
    std::shared_ptr<Connection> conn_;
    size_t expired_time_;
    bool deleted_;
};
struct TimerCmp {
    bool operator()(std::shared_ptr<TimerNode>& a, std::shared_ptr<TimerNode>& b) {
        return a->getExpTime() > b->getExpTime();
    }
};
class TimerManager {
public:
    TimerManager();
    ~TimerManager();
    std::shared_ptr<TimerNode> addTimer(std::shared_ptr<Connection> conn, int timeout);
    void handleExpiredEvent();
    std::mutex mutex_;
private:
    std::priority_queue<std::shared_ptr<TimerNode>, std::deque<std::shared_ptr<TimerNode>>, TimerCmp>
        timerNodeQueue;
};