#include "Platform.hpp"

using namespace hg::engine;
using namespace hg;

std::atomic<bool>& platform::dragActive() noexcept {
	static std::atomic<bool> active { false };
	return active;
}

Platform::Platform() noexcept = default;

Platform::~Platform() = default;
