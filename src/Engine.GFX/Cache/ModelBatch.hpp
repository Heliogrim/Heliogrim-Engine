#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg::engine::gfx::cache {
	// Warning: Do NOT instantiate ModelBatch itself (it might fail anyway cause of `novtable` annotation)
	struct __declspec(novtable) ModelBatch :
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
