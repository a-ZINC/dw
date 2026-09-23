#include "../include/dw/proc_self.h"
#include <cstdio>
#include <map>

int main() {
    dw::MemSnapshot mem;
    std::vector<dw::MapEntry> maps;
    if (!dw::read_mem_snapshot(mem) || !dw::read_map_snapshot(maps)) {
        std::fprintf(stderr, "error reading status and maps");
        return 1;
    }

    struct group {
        int regions;
        size_t kbs;
    };
    std::map<std::string, group> groups;
    size_t total_size = 0;

    for (auto& m: maps) {
        std::string category;
        if(m.path.empty()) {
            category = "anon";
        } else if (m.path.at(0) == '[') {
            category = m.path;
        } else {
            category = "file backed";
        }
        groups[category].regions += 1;
        groups[category].kbs += m.size_kb(m.start, m.end);
        total_size += m.size_kb(m.start, m.end);
    }

    std::printf("%-14s %8s %12s\n", "category", "regions", "size(kB)");
    for (auto& [name, group]: groups) {
        std::printf("%-14s %8d %12lu\n", name.c_str(), group.regions, group.kbs);
    }

    std::printf("\nsum of all mappings : %lu kB\n", total_size);
    std::printf("VmSize (status)     : %ld kB\n", mem.vm_size_kb);
    std::printf("difference          : %ld kB\n", static_cast<long>(total_size) - mem.vm_size_kb);


}