#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "TransientCache.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx::cache {
	class LocalResourceCache final {
	public:
		friend class LocalCacheCtrl;

	public:
		using this_type = LocalResourceCache;

	public:
		LocalResourceCache(const nmpt<GlobalCacheCtrl> global_);

		LocalResourceCache(cref<this_type>) = delete;

		LocalResourceCache(mref<this_type> other_) noexcept;

		~LocalResourceCache();

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type> other_) noexcept;

	public:
		void tidy();

	private:
		nmpt<GlobalCacheCtrl> _global;

	public:
		void reset();

		void reset(const bool fully_);

	private:
		std::array<TransientCache, 2> _caches;
		std::array<ptr<TransientCache>, 2> _shifting;

	public:
		[[nodiscard]] CacheResult fetch(cref<std::ptrdiff_t> key_, _Out_ ref<ptr<ModelBatch>> dst_);

		void store(cref<std::ptrdiff_t> key_, mref<ptr<ModelBatch>> obj_);

	private:
		void shift();
	};
}
