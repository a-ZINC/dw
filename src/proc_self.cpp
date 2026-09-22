#include "../include/dw/proc_self.h"
#include "../include/dw/fd.h"

#include <cerrno>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <string_view>
#include <string>
#include <system_error>
#include <sys/resource.h>

namespace dw {
    bool read_file(const char *path, std::string &output) {
        output.clear();
        Fd fd(open(path, O_RDONLY | O_CLOEXEC));
        if (!fd.valid()) {
            perror("open!");
            return false;
        }

        char buffer[4096];
        while(true) {
            size_t n = read(fd.get(), &buffer, sizeof(buffer));
            if (n) {
                output.append(buffer, static_cast<int>(n));
            } else if (n == 0) {
                return true;
            } else if (errno != EINTR) {
                return false;
            }
        }
    }

    std::int64_t parse_number(std::string_view s) {
        while(!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
            s.remove_prefix(1);
        }
        std::int64_t value = -1;
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
        return ec == std::errc{} ? value : -1;

    }

    bool read_mem_snapshot(MemSnapshot& mem_snapshot) {
        mem_snapshot = {};

        std::string mem_snapshot_buffer;
        if (!read_file("/proc/self/status", mem_snapshot_buffer)) {
            return false;
        }

        std::string_view mem_snapshot_view{mem_snapshot_buffer};
        while(!mem_snapshot_view.empty()) {
            size_t n = mem_snapshot_view.find("\n");
            std::string_view line = mem_snapshot_view.substr(0, n);
            mem_snapshot_view = (n == std::string_view::npos) ? std::string_view{} : mem_snapshot_view.substr(n+1);

            // fetch key
            size_t key_size = line.find(":");
            if (key_size == std::string_view::npos) continue;
            std::string_view key = line.substr(0, key_size);
            std::string_view value = line.substr(key_size + 1);

            if (key == "VmSize") {
                mem_snapshot.vm_size_kb = parse_number(value);
            } else if (key == "VmRSS") {
                mem_snapshot.vm_rss_kb = parse_number(value);
            } else if (key == "VmHWM") {
                mem_snapshot.vm_hwm_kb = parse_number(value);
            } else if (key == "RssAnon") {
                mem_snapshot.rss_anon_kb = parse_number(value);
            } else if (key == "RssFile") {
                mem_snapshot.rss_file_kb = parse_number(value);
            } else if (key == "RssShmem") {
                mem_snapshot.rss_shmem_kb = parse_number(value);
            } else if (key == "Threads") {
                mem_snapshot.threads = parse_number(value);
            }
        }

        rusage ru{};
        if (getrusage(RUSAGE_SELF, &ru)) {
            mem_snapshot.minor_faults = ru.ru_minflt;
            mem_snapshot.major_faults = ru.ru_majflt;
        }
        return true;
    }

    bool read_map_snapshot(std::vector<MapEntry>& map_entries) {
        map_entries = {};
        std::string output;
        if (!read_file("/proc/self/maps",output)) {
            return false;
        }

        std::string_view map_snapshot_view{output};
        while(!map_snapshot_view.empty()) {
            size_t n = map_snapshot_view.find("\n");
            std::string_view line = map_snapshot_view.substr(0, n);
            map_snapshot_view = (n == std::string_view::npos) ? std::string_view{} : map_snapshot_view.substr(n+1);

            std::uint64_t start;
            std::uint64_t end;
            std::uint64_t offset;
            std::uint64_t inode;
            char perm[8];
            char dev[32];
            int consumed = -1;
            int fields = std::sscanf(line.data(), "%lx-%lx %7s %lx %31s %lx %n", &start, &end, perm, &offset, dev, &inode, &consumed);
            if (fields < 6) return false;

            MapEntry e;
            e.start = start;
            e.end = end;
            e.offset = offset;
            e.inode = inode;
            e.permission = perm;

            if (consumed >= 0 && static_cast<size_t>(consumed) < line.size()) {
                e.path = line.substr(static_cast<size_t>(consumed));
            }

            map_entries.push_back(e);
        }

        return true;
    }

    const MapEntry* find_entry(std::vector<MapEntry>& mapEntry, void* addr) {
        std::uint64_t address = reinterpret_cast<int64_t>(addr);
        for (auto& m : mapEntry) {
            if (address >= m.start && address <= m.end) {
                return &m;
            }
        }
        return nullptr;
    }


}

