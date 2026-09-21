#pragma once

#include <atomic>
#include <cstdint>
#include<string>
#include <vector>

namespace dw {
    bool read_file(const char* path, std::string& output);

    struct MemSnapshot {
        std::int64_t vm_size_kb = -1;
        std::int64_t vm_rss_kb = -1;
        std::int64_t vm_hwm_kb = -1;
        std::int64_t rss_anon_kb = -1;
        std::int64_t rss_file_kb = -1;
        std::int64_t rss_shmem_kb = -1;
        std::int32_t threads = -1;
        std::int64_t minor_faults = -1;
        std::int64_t major_faults = -1;
    };

    bool read_mem_snapshot(MemSnapshot& mem_snapshot);

    struct MapEntry {
        std::int64_t start = 0;
        std::int64_t end = 0;
        std::string permission;
        std::int64_t offset = 0;
        std::int64_t inode = 0;
        std::string path;

        std::int64_t size_kb(std::int64_t start, int64_t end) {
            return (end -  start) / 1024;
        }
    };

    bool read_map_snapshot(std::vector<MapEntry>& map_entries); 
}