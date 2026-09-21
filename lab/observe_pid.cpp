// Watch VmSize, VmRSS and the open-descriptor count of a running process.
// Usage: observe_pid <pid> [samples=10] [interval_ms=1000]
//
// A first, deliberately simple piece of Debugging Wizard: once per sample it
// reads /proc/<pid>/status and counts the entries of /proc/<pid>/fd.
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>

#include "../include/dw/fd.h"

namespace {

struct Sample {
    long vm_size_kb = -1;  // -1 means "field not present"
    long vm_rss_kb = -1;
    long fd_count = -1;
};

// Find "Key:   1234 kB" in the text and return 1234, or -1 if absent.
long find_kb(const char* text, const char* key) {
    const char* p = std::strstr(text, key);
    if (p == nullptr) {
        return -1;
    }
    long value = -1;
    if (std::sscanf(p + std::strlen(key), "%ld", &value) != 1) {
        return -1;
    }
    return value;
}

// Returns false if the process is gone or we lack permission.
bool read_status(pid_t pid, Sample& out) {
    char path[64];
    std::snprintf(path, sizeof path, "/proc/%d/status", static_cast<int>(pid));

    dw::Fd fd(::open(path, O_RDONLY));
    if (!fd.valid()) {
        return false;
    }
    char buf[8192];
    ssize_t n = ::read(fd.get(), buf, sizeof buf - 1);  // one read: see Chapter 2 for why this is naive
    if (n <= 0) {
        return false;
    }
    buf[n] = '\0';
    out.vm_size_kb = find_kb(buf, "VmSize:");
    out.vm_rss_kb = find_kb(buf, "VmRSS:");
    return true;
}

long count_fds(pid_t pid) {
    char path[64];
    std::snprintf(path, sizeof path, "/proc/%d/fd", static_cast<int>(pid));
    std::error_code ec;
    long n = 0;
    for (std::filesystem::directory_iterator it(path, ec), end; !ec && it != end; it.increment(ec)) {
        ++n;
    }
    return ec ? -1 : n;
}

double elapsed_s(const timespec& start) {
    timespec now{};
    clock_gettime(CLOCK_MONOTONIC, &now);
    return static_cast<double>(now.tv_sec - start.tv_sec) +
           static_cast<double>(now.tv_nsec - start.tv_nsec) / 1e9;
}

void add_ms(timespec& t, long ms) {
    t.tv_sec += ms / 1000;
    t.tv_nsec += (ms % 1000) * 1000000L;
    if (t.tv_nsec >= 1000000000L) {
        t.tv_sec += 1;
        t.tv_nsec -= 1000000000L;
    }
}

// Sleep until an ABSOLUTE point in time, so errors do not accumulate.
void sleep_until(const timespec& deadline) {
    while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, nullptr) == EINTR) {
        // interrupted by a signal: resume waiting for the same deadline
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <pid> [samples=10] [interval_ms=1000]\n", argv[0]);
        return 2;
    }
    const pid_t pid = static_cast<pid_t>(std::atoi(argv[1]));
    const int samples = (argc > 2) ? std::atoi(argv[2]) : 10;
    const long interval_ms = (argc > 3) ? std::atol(argv[3]) : 1000;
    if (pid <= 0 || samples < 1 || interval_ms < 1) {
        std::fprintf(stderr, "invalid argument\n");
        return 2;
    }

    timespec start{};
    clock_gettime(CLOCK_MONOTONIC, &start);
    timespec deadline = start;

    std::printf("%-9s %-12s %-12s %-6s\n", "t(s)", "VmSize(kB)", "VmRSS(kB)", "fds");
    for (int i = 0; i < samples; ++i) {
        Sample s;
        if (!read_status(pid, s)) {
            std::fprintf(stderr, "cannot read /proc/%d/status (process gone or no permission)\n",
                         static_cast<int>(pid));
            return 1;
        }
        s.fd_count = count_fds(pid);
        std::printf("%-9.3f %-12ld %-12ld %-6ld\n", elapsed_s(start), s.vm_size_kb, s.vm_rss_kb,
                    s.fd_count);
        add_ms(deadline, interval_ms);  // absolute deadline: no cumulative drift
        sleep_until(deadline);
    }
    return 0;
}