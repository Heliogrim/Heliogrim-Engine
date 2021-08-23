#pragma once
#include <ctime>
#include <mutex>

namespace ember {
	static _STD chrono::high_resolution_clock::time_point processStartTime = _STD chrono::high_resolution_clock::now();
}

#ifdef _DEBUG

namespace ember::debug {
	static _STD recursive_mutex outMtx {};
}

template<typename Ty>
void printSet(const Ty*& ptr_, const size_t count_)
{
	if (count_ < 1)
	{
		std::cout << "[ ]" << std::endl;
	}

	std::stringstream ss = std::stringstream();

	// Open
	ss << "[ ";

	for (size_t i = 1; i < count_; i ++)
		ss << ", " << ptr_[i];

	// Close
	ss << " ]";

	std::cout << ss.str() << std::endl;
}

#define DEBUG_OUT(x) do { ember::debug::outMtx.lock(); SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); std::cout << _STD to_string(_STD chrono::duration_cast<_STD chrono::milliseconds>(_STD chrono::high_resolution_clock::now() - ember::processStartTime).count()) << " | " << x << std::endl; ember::debug::outMtx.unlock(); } while (0);
#define DEBUG_SNMSG(s, n, msg) do { ember::debug::outMtx.lock(); if ((s)) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2); } else { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4); } std::cout << ( (s) ? " # " : " x " ) << std::flush; SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); DEBUG_OUT("[ " << (n) << " ] " << (msg)); ember::debug::outMtx.unlock(); } while (0);
#define DEBUG_NMSG(n, msg) DEBUG_SNMSG(true,n,msg)
#define DEBUG_MSG(msg) DEBUG_NMSG("LOG",msg)

#else

#ifndef NDEBUG
#define NDEBUG 1
#endif

#define DEBUG_OUT(x)
#define DEBUG_MSG(x)
#define DEBUG_NMSG(n, msg)
#define DEBUG_SNMSG(s, n, msg)
#define DEBUG_SET_MSG(set)

#endif
