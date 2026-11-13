#pragma once

#include "IoBackend.hpp"

namespace hg::engine::io {
	class IocpBackend final :
		public IoBackend {
	public:
		IocpBackend(ref<IoThread> thread_, sink_type sink_);

	public:
		[[nodiscard]] u32 associate(ptr<void> handle_) override;

		void issue(ref<const IoRequest> request_, ptr<void> token_) override;

		void harvest(ref<const IoPortEntry> entry_) override;

	private:
		ref<IoThread> _thread;
		sink_type _sink;
	};
}
