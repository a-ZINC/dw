#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <sys/mman.h>
#include "../include/dw/proc_self.h"

#define CHECK(cond)                                                                       \
    do {                                                                                  \
        if (!(cond)) {                                                                    \
            std::fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__);\
            std::exit(1);                                                                 \
        }                                                                                 \
    } while(0)      
    
#define CHECK_PRINT(cond, prin)                                                                       \
    do {                                                                                  \
        if (!(cond)) {                                                                    \
            std::fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__);\
            std::exit(1);                                                                 \
        } else {                                                                           \
            std::cout << "CHECK: " << #cond << "  output: " << prin << std::endl;                                  \
        }                                                                                \
    } while(0)   

int main() {
    {   
        dw::Result<std::string> output_result = dw::read_file("/proc/self/status");
        CHECK(output_result.ok());
        CHECK(output_result.value().size() > 0);
        CHECK(output_result.value().find("VmRSS") != std::string::npos);
        CHECK(!dw::read_file("/proc/this/does/not/exist"));
    }

    {
        dw::MemSnapshot ms;
        CHECK(dw::read_mem_snapshot(ms));
        CHECK(ms.vm_rss_kb > 0);
        CHECK((ms.rss_anon_kb + ms.rss_file_kb + ms.rss_shmem_kb) == ms.vm_rss_kb);
    }

    {
        std::vector<dw::MapEntry> entries;
        CHECK(dw::read_map_snapshot(entries));
        CHECK_PRINT(entries.size() > 0, entries[0].start);

        const dw::MapEntry* stack = dw::find_entry(entries, __builtin_frame_address(0));
        CHECK(stack != nullptr && stack->path == "[stack]");

        void* anon = mmap(nullptr, 1 << 20, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        CHECK(anon != MAP_FAILED);
        CHECK(dw::read_map_snapshot(entries));
        const dw::MapEntry* anon_vma = dw::find_entry(entries, anon);
        dw::MapEntry anon_defrence = anon_vma != nullptr ? *anon_vma : dw::MapEntry{};
        CHECK(anon_vma != nullptr && anon_vma->path.empty() && anon_vma->permission == "rw-p");
        munmap(anon, 1<<20);

        const dw::MapEntry* fun_vma = dw::find_entry(entries, reinterpret_cast<void*>(&dw::read_map_snapshot));
        CHECK(fun_vma != nullptr);
        CHECK(fun_vma->permission.size() > 2 && fun_vma->permission[2] == 'x');
    }

    std::puts("all test case passed");
    return 0;
}