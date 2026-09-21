#include <fcntl.h>
#include <cstdio>
#include "../include/dw/fd.h"

int main() {
    for (int i=0;; i++) {
        dw::Fd fd(open("/proc/meminfo", O_RDONLY));
        if (!fd.valid()) {
            std::perror("open");
            return 1;
        }
        std::puts("done: no fd leak");
        return 0;
    }
}