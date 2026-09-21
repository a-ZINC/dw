#pragma once

#include<unistd.h>

namespace dw {
    class Fd {
    public:
        Fd() noexcept = default;
        Fd(int fd) noexcept : fd_(fd) {}

        Fd(const Fd&) = delete;
        Fd& operator=(const Fd&) = delete;

        Fd(Fd&& other) : fd_(other.release()) {}
        Fd& operator=(Fd&& other) {
            if (other.fd_ != -1) {
                reset(other.release());
            }
            return *this;
        }

        int get() const noexcept { return fd_; }
        bool valid() const noexcept { return fd_ >= 0; }

        int release() {
            int f = fd_;
            fd_ = -1;
            return f;
        }

        void reset(int fd) {
            if(fd_ >= 0) {
                close(fd_);
            }
            fd_ = fd;
        }
    private:
        int fd_;
    };
}