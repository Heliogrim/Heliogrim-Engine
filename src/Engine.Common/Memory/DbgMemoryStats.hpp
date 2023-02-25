#pragma once

#include "../stdafx.h"
#include <Engine.Logging/Logger.hpp>
#include <unordered_map>
#include <algorithm>
#ifdef DEBUG_CPP
#include <mutex>
#endif

namespace ember::debug {
    struct MemoryStats {
        intptr_t allocated = 0;
        uint32_t flags = 0;

        std::vector<void*> ptrs = std::vector<void*>();
    };

    class DbgMemoryStats {
    public:
        static void updateAllocated(
            const std::string& key_,
            const size_t size_,
            const uint32_t& flags_,
            void* ptr_ = nullptr
        ) {
            const auto entry = _stats.find(key_);

            if (entry != _stats.end()) {
                auto stats = std::find_if(
                    entry->second.begin(),
                    entry->second.end(),
                    [flags_](const MemoryStats& s_) {
                        return (s_.flags == flags_);
                    }
                );

                if (stats == entry->second.end()) {
                    entry->second.push_back({ 0, flags_ });

                    stats = std::find_if(
                        entry->second.begin(),
                        entry->second.end(),
                        [flags_](const MemoryStats& s_) {
                            return (s_.flags == flags_);
                        }
                    );
                }

                stats->allocated += size_;
                if (ptr_)
                    stats->ptrs.push_back(ptr_);
            } else {
                std::vector<MemoryStats> list = std::vector<MemoryStats>();
                list.push_back({ 0, flags_ });

                MemoryStats& stats = list[0];

                stats.allocated += size_;
                if (ptr_)
                    stats.ptrs.push_back(ptr_);

                _stats.insert(std::make_pair(key_, list));
            }

            updateDisplay();
        }

        static void updateFree(
            const std::string& key_,
            const size_t size_,
            const uint32_t& flags_,
            void* ptr_ = nullptr
        ) {
            const auto entry = _stats.find(key_);

            if (entry == _stats.end())
                return;

            auto stats = std::find_if(
                entry->second.begin(),
                entry->second.end(),
                [flags_](const MemoryStats& s_) {
                    return (s_.flags == flags_);
                }
            );

            if (stats == entry->second.end())
                return;

            if (ptr_) {
                const auto pptr = std::find(stats->ptrs.begin(), stats->ptrs.end(), ptr_);
                if (pptr == stats->ptrs.end()) {
                    IM_DEBUG_LOG("Free memory block which is not tracked by this pointer.");
                } else {
                    stats->ptrs.erase(pptr);
                }
            }

            stats->allocated -= static_cast<intptr_t>(size_);

            if (stats->allocated < 0) {
                IM_DEBUG_LOG("More memory freeed than allocated. Seam to have untracked memory.");
            }

            updateDisplay();
        }

        static void updateDisplay() {
            #ifdef DEBUG_CPP
			std::unique_lock<std::mutex> lock(_mtx);
            #endif

            IM_DEBUG_LOG("");
            IM_DEBUG_LOG("##### Memory Stats Tracking #####");

            for (const auto& mapEntry : _stats) {
                const std::string key = mapEntry.first;

                for (const auto& listEntry : mapEntry.second) {
                    const MemoryStats& stats = listEntry;

                    std::string suffix = "B";
                    double size = static_cast<double>(stats.allocated);

                    if (size / 1024.0 >= 1.0) {
                        size /= 1024;
                        suffix = "KB";
                    }

                    if (size / 1024.0 >= 1.0) {
                        size /= 1024;
                        suffix = "MB";
                    }

                    if (size / 1024.0 >= 1.0) {
                        size /= 1024;
                        suffix = "GB";
                    }

                    IM_DEBUG_LOGF(
                        R"( {} -> [ Flags: {} Size: {}{} ({})])",
                        key,
                        stats.flags,
                        size,
                        suffix,
                        stats.allocated
                    );
                }
            }
        }

    private:
        #ifdef DEBUG_CPP
		static std::mutex _mtx;
        #endif
        static std::unordered_map<std::string, std::vector<MemoryStats>> _stats;
    };
}
