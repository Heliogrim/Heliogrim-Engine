#pragma once

#include <atomic>

namespace hg {
	// Only for the deprecated await overload; removed next release.
	using await_signal_type = std::atomic_flag;
	using await_signal_sub_type = const std::atomic_flag;
}
