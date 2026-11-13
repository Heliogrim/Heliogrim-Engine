#pragma once

#include <functional>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "IoThread.hpp"
#include "IoTypes.hpp"

namespace hg::engine::io {
	enum class IoBackendKind : u8 {
		eIocp,
		eBlocking
	};

	// All members run on the io thread only.
	class IoBackend {
	public:
		// Receives the token of the finished request; may be called inline from issue().
		using sink_type = std::function<void(ptr<void>, IoCompletion)>;

	public:
		virtual ~IoBackend() = default;

	public:
		// The CreateFileW flags files for this backend must be opened with.
		[[nodiscard]] virtual u32 openFlags() const noexcept = 0;

		// Returns 0 or the OS error.
		[[nodiscard]] virtual u32 associate(ptr<void> handle_) = 0;

		virtual void issue(ref<const IoRequest> request_, ptr<void> token_) = 0;

		virtual void harvest(ref<const IoPortEntry> entry_) = 0;

		// Asks an issued request to finish early via CancelIoEx; it still reaches the sink, as eCancelled if no bytes landed, else eShort or eDone.
		virtual void cancel(ptr<void> token_) = 0;

		// cancel() for every issued request.
		virtual void cancelAll() = 0;

	protected:
		// Maps an OS result to a completion; size_ is the requested byte count.
		[[nodiscard]] static IoCompletion translate(const u32 error_, const u32 bytes_, const u32 size_) noexcept;

	public:
		[[nodiscard]] static uptr<IoBackend> make(const IoBackendKind kind_, ref<IoThread> thread_, sink_type sink_);
	};
}
