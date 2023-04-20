#include"Buffer.h"
#include"util.h"
const std::string& Buffer::GetBuf() {
    return buf_;
}
const char* Buffer::c_str() {
    return buf_.c_str();
}
void Buffer::SetBuf(const char* str) {
    std::string new_buf(str);
    buf_.swap(new_buf);
}
size_t Buffer::GetSize() {
    DEBUG("size=%d\n", buf_.size());
    return buf_.size();
}
void Buffer::Append(const char* str, int size) {
    for (int i = 0; i < size; ++i) {
        //if (str[i] == '\0')break;
        buf_ += str[i];
    }
}
void Buffer::Clear() {
    buf_.clear();
}
int Buffer::Find(const char* s, int pos) {
    auto ret = buf_.find_first_of(s, pos);
    if (ret != std::string::npos) {
        return ret;
    }
    else return -1;
}
std::string Buffer::GetContext(int l, int r) {
    return buf_.substr(l, r - l + 1);
}