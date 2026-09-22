
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/mman.h>
#include "../include/dw/proc_self.h"

void row(std::string label, const dw::MemSnapshot& mem, dw::MemSnapshot& base) {
    std::printf("%-31s %10ld %10ld %10ld %10ld %12ld\n", label.c_str(), mem.vm_size_kb, mem.vm_rss_kb, mem.rss_anon_kb, mem.vm_hwm_kb, mem.minor_faults - base.minor_faults);
}

dw::MemSnapshot snap() {
    dw::MemSnapshot snapshot;
    if (!dw::read_mem_snapshot(snapshot)) {
        std::fprintf(stderr, "cannot read /proc/self/status\n");
        std::exit(1);
    }
    return snapshot;
}

int main(int argc, char** argv) {
    long size_mb = (argc > 1) ? std::atoi(argv[1]) : 256;
    if (size_mb < 4) {
        std::fprintf(stderr, "usage: %s [megabytes>=4]\n", argv[0]);
        return 2;
    }

    std::size_t size = static_cast<size_t>(size_mb * 1024 * 1024);
    size_t page = static_cast<size_t>(sysconf(_SC_PAGESIZE));

    std::string thp;
    if(dw::read_file("/sys/kernel/mm/transparent_hugepage/enabled", thp)) {
        std::fprintf(stdin, "huge table enabled?: %s\n", thp.c_str());
    }
    std::printf("page size: %zu bytes, region: %ld MB = %zu pages\n\n", page, size_mb, size / page);
    std::printf("%-30s %10s %10s %10s %10s %12s\n", "step", "VmSize kB", "VmRSS kB", "RssAnon kB",
                "VmHWM kB", "minflt delta");

    dw::MemSnapshot base = snap();
    row("0: Base", base, base);

    void* allotment = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (allotment == MAP_FAILED) {
        std::printf("error in allocating!\n");
        return 1;
    }
    volatile char* untouched_mem = static_cast<volatile char*>(allotment);
    row("1: after mmap(untouched)", snap(), base);

    for (std::size_t off = 0; off < size / 2; off += page) {
        untouched_mem[off] = 1;
    }
    row("2: touched first half", snap(), base);

    for (std::size_t off = (size/2); off < size; off += page) {
        untouched_mem[off];
    }
    row("3: touched everything", snap(), base);

    if (madvise(allotment, size, MADV_DONTNEED)) {
        std::perror("madvise");
        return 1;
    }
    row("4: madvise(DONTNEED)", snap(), base);

    for (std::size_t off = 0; off < size / 2; off += page) {
        untouched_mem[off] = 1;
    }
    row("5: retouched first half", snap(), base);

    munmap(allotment, size);
    row("6: munmap", snap(), base);
    return 0;
}