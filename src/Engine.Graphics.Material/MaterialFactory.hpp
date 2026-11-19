#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>

#include "MaterialParameter.hpp"
#include "MaterialPrototypeParameter.hpp"
#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::material {
	class MaterialFactory {
	public:
		[[nodiscard]] _Success_(return != nullptr) uptr<Material> buildMaterial(
			mref<smr<MaterialPrototype>> prototype_
		) const;

	public:
		[[nodiscard]] MaterialParameter buildMaterialParameter(cref<MaterialPrototypeParameter> prototype_) const;

		[[nodiscard]] MaterialPrototypeParameter buildMaterialPrototypeParameter(
			mref<ParameterIdentifier> identifier_,
			mref<string> name_,
			mref<accel::TransferDataType> dataType_
		) const;

	protected:
		[[nodiscard]] _Success_(return != nullptr) uptr<MaterialParameterStorageBase> buildMaterialParameterStorage(
			accel::TransferDataType dataType_
		) const;
	};
}
