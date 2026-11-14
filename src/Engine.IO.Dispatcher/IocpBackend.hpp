#pragma once

#include <Engine.Common/Collection/DenseMap.hpp>

#include "IoBackend.hpp"

namespace hg::engine::io {
	class IocpBackend final :
		public IoBackend {
	public:
		IocpBackend(ref<IoThread> thread_, sink_type sink_);

		~IocpBackend() override;

	public:
		[[nodiscard]] u32 openFlags() const noexcept override;

		[[nodiscard]] u32 associate(ptr<void> handle_) override;

		void issue(ref<const IoRequest> request_, ptr<void> token_) override;

		void harvest(ref<const IoPortEntry> entry_) override;

		void cancel(ptr<void> token_) override;

		void cancelAll() override;

	private:
		struct Slot;

	private:
		ref<IoThread> _thread;
		sink_type _sink;
		// Token to its OVERLAPPED slot while the request awaits its packet; heap-owned, so the address stays stable.
		DenseMap<ptr<void>, uptr<Slot>> _slots;
	};
}
