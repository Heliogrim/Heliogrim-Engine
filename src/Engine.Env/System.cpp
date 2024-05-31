#include "System.hpp"

#include <Engine.Common/stdafx.h>

using namespace hg::env;

#if _WIN32 || _WIN64

#include <sysinfoapi.h>

size_t hg::env::get_memory_page_size() {
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwPageSize;
}

#elif __linux__ || __unix__ || defined(_POSIX_SOURCE)

#include <unistd.h>

size_t hg::env::get_memory_page_size() {
	return sysconf(_SC_PAGESIZE);
}

#else

size_t hg::env::get_memory_page_size() {
	size_t n;
	char* ptr;
	int u;

	for (n = 1; n; n *= 2) {
		ptr = mmap(0, n*2, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

		if (ptr == MAP_FAILED) {
			return -1;
		}

		u = munmap(p + n, n);

		if (!u) {
			return n;
		}
	}
}

#endif
