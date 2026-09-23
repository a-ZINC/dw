#include "../include/dw/proc_self.h"
#include <bits/types/struct_timeval.h>
#include <chrono>
#include <cstdio>
#include <sys/resource.h>

rusage self_usage() {
    rusage ru{};
    ::getrusage(RUSAGE_SELF, &ru);
    return ru;
}

double to_seconds(timeval& time_val) {
    return static_cast<double>(time_val.tv_sec) + (static_cast<double>(time_val.tv_usec)/ 1e6);
}


int main(int argc, char** argv) {
    int times = (argc > 1) ? std::atol(argv[1]) : 20000;
    size_t sink = 0;
    rusage before_loop = self_usage();
    auto start = std::chrono::steady_clock::now();
    for (int i=0; i< times; i++) {
        dw::MemSnapshot mem_snapshot;
        if(!dw::read_mem_snapshot(mem_snapshot)) {
            std::fprintf(stderr, "unable to read status");
            return 1;
        }
        sink += mem_snapshot.vm_size_kb;
    }
    auto status_time = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    rusage after_loop = self_usage();

    size_t x = sink * (times/100);
    auto user_time = to_seconds(after_loop.ru_utime) - to_seconds(before_loop.ru_utime);
    auto sys_time = to_seconds(after_loop.ru_stime) - to_seconds(before_loop.ru_stime);
    long minflt = after_loop.ru_minflt - before_loop.ru_minflt;

    std::printf("calls=%d\n", times);
    std::printf("wall total = %.4f s   (%.2f us/call)\n", status_time, status_time / static_cast<double>(times) * 1e6);
    std::printf("user total = %.4f s   (%.2f us/call)\n", user_time, user_time / static_cast<double>(times) * 1e6);
    std::printf("sys  total = %.4f s   (%.2f us/call)\n", sys_time, sys_time / static_cast<double>(times) * 1e6);
    std::printf("minor faults during loop = %ld\n", minflt);
    std::printf("(sink=%ld, x=%ld, ignore: keeps the compiler from deleting the loop)\n", sink, x);

}

