#include <cstdio>
#include <unistd.h>

int main() {
    std::printf("hello from pid %d\n", static_cast<int>(getpid()));
    return 0;
}