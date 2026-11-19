#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::cache {
	// Warning: Do NOT instantiate ModelBatch itself (it might fail anyway cause of `novtable` annotation)
	struct macro_novtable ModelBatch :
		public InheritBase<ModelBatch> {
	public:
		~ModelBatch() override = default;

	public:
		ref<ModelBatch> operator=(cref<ModelBatch>) = delete;

		ref<ModelBatch> operator=(mref<ModelBatch>) noexcept = default;

	public:
		virtual ptr<ModelBatch> clone() = 0;
	};
}
