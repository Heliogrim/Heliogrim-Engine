#include "DbgMemoryStats.hpp"

#ifdef DEBUG_CPP
std::mutex DbgMemoryStats::_mtx = std::mutex();
#endif
_STD unordered_map<std::string, std::vector<ember::debug::MemoryStats>> ember::debug::DbgMemoryStats::_stats = _STD unordered_map<std::string, std::vector<ember::debug::MemoryStats>>();