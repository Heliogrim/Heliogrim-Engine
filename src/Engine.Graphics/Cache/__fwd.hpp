#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::cache {
	class GlobalCacheCtrl;
	class LocalCacheCtrl;
	class GlobalResourceCache;
	class LocalResourceCache;
	class TransientCache;

	struct ModelBatch;

	enum CacheResult: u8;

	template <std::equality_comparable SubjectType_>
	class CacheCtrlSubject;

	struct CacheStaticGeometrySubject;
	struct CacheTextureSubject;
}
