#include <chrono>
#include <cstring>
#include <thread>
#include<unistd.h>
#include<fcntl.h>
#include<cerrno>
#include <cstdio>


int main() {
    for (int i=0;; i++) {
        int fd = open("/proc/meminfo", O_RDONLY);
        if (fd < 0) {
            std::printf("open failed at iteration %d: %s\n", i, std::strerror(errno));
            return 1;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}