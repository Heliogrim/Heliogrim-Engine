#pragma once

#include "IoBackend.hpp"

namespace hg::engine::io {
	// Runs each request synchronously on the io thread; a baseline for the iocp backend.
	class BlockingBackend final :
		public IoBackend {
	public:
		explicit BlockingBackend(sink_type sink_);

	public:
		[[nodiscard]] u32 openFlags() const noexcept override;

		[[nodiscard]] u32 associate(ptr<void> handle_) override;

		void issue(ref<const IoRequest> request_, ptr<void> token_) override;

		void harvest(ref<const IoPortEntry> entry_) override;

		void cancel(ptr<void> token_) override;

		void cancelAll() override;

	private:
		sink_type _sink;
	};
}
