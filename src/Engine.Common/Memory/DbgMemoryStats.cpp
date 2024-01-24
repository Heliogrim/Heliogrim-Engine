#include "DbgMemoryStats.hpp"

#ifdef DEBUG_CPP
std::mutex DbgMemoryStats::_mtx = std::mutex();
#endif
std::unordered_map<std::string, std::vector<hg::debug::MemoryStats>> hg::debug::DbgMemoryStats::_stats =
    std::unordered_map<std::string, std::vector<hg::debug::MemoryStats>>();
