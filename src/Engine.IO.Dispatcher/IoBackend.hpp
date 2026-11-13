#pragma once

#include <functional>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "IoThread.hpp"
#include "IoTypes.hpp"

namespace hg::engine::io {
	enum class IoBackendKind : u8 {
		eIocp
	};

	// All members run on the io thread only.
	class IoBackend {
	public:
		// Receives the token of the finished request; may be called inline from issue().
		using sink_type = std::function<void(ptr<void>, IoCompletion)>;

	public:
		virtual ~IoBackend() = default;

	public:
		[[nodiscard]] static uptr<IoBackend> make(IoBackendKind kind_, ref<IoThread> thread_, sink_type sink_);

	public:
		// Returns 0 or the OS error.
		[[nodiscard]] virtual u32 associate(ptr<void> handle_) = 0;

		virtual void issue(ref<const IoRequest> request_, ptr<void> token_) = 0;

		virtual void harvest(ref<const IoPortEntry> entry_) = 0;
	};
}
