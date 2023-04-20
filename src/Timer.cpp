#include"Timer.h"
TimerNode::TimerNode(std::shared_ptr<Connection> conn, int timeout) {
    deleted_ = false;
    conn_ = conn;
    expired_time_ = time(0) + timeout;
}
TimerNode::~TimerNode() {
    //todo
}
TimerNode::TimerNode(const TimerNode& tn) {
    deleted_ = tn.deleted_;
    conn_ = tn.conn_;
    expired_time_ = tn.expired_time_;
}
void TimerNode::update(int timeout) {
    expired_time_ = time(0) + timeout;
}
bool TimerNode::isValid() {
    size_t now = time(0);
    if (now >= expired_time_) {
        return false;
    }
    else return true;
}
void TimerNode::setDeleted() {
    deleted_ = true;
}
bool TimerNode::isDeleted() {
    return deleted_;
}

size_t TimerNode::getExpTime() {
    return expired_time_;
}
TimerManager::TimerManager() {}
TimerManager::~TimerManager() {}
std::shared_ptr<TimerNode> TimerManager::addTimer(std::shared_ptr<Connection>conn, int timeout) {
    std::lock_guard<std::mutex>lock(mutex_);
    std::shared_ptr<TimerNode> time_node = std::make_shared<TimerNode>(conn, timeout);
    conn->SetTimerNode(time_node);
    timerNodeQueue.push(time_node);
    return time_node;
}
void TimerManager::handleExpiredEvent() {
    std::lock_guard<std::mutex>lock(mutex_);
    while (!timerNodeQueue.empty()) {
        std::shared_ptr<TimerNode> timer_node = timerNodeQueue.top();
        //printf("now:%ld,timernode:%ld\n", time(0), timer_node->getExpTime());
        if (timer_node->isDeleted())
            timerNodeQueue.pop();
        else if (timer_node->isValid() == false) {
            timer_node->setDeleted();
            timerNodeQueue.pop();
            //printf("close timernode \n");
            timer_node->GetConnection()->Close();
        }
        else break;
    }

}