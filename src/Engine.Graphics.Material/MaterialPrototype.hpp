#pragma once

#include "__fwd.hpp"
#include "EffectUsagePattern.hpp"
#include "MaterialEffect.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::material {
	class MaterialPrototype {
	public:
		using this_type = MaterialPrototype;

	public:
		MaterialPrototype() noexcept = default;

		MaterialPrototype(
			mref<Guid> guid_,
			mref<string> name_,
			mref<InlineAutoArray<MaterialEffect>> effects_,
			mref<Vector<MaterialPrototypeParameter>> parameters_
		) noexcept;

		MaterialPrototype(mref<this_type> other_) noexcept = default;

		~MaterialPrototype();

	private:
		Guid _guid;

	public:
		[[nodiscard]] Guid getGuid() const noexcept;

	private:
		string _name;

	public:
		[[nodiscard]] string getName() const noexcept;

	private:
		InlineAutoArray<MaterialEffect> _materialEffects;

	public:
		[[nodiscard]] cref<InlineAutoArray<MaterialEffect>> getMaterialEffects() const noexcept;

	private:
		Vector<MaterialPrototypeParameter> _parameters;

	public:
		[[nodiscard]] cref<Vector<MaterialPrototypeParameter>> getParameters() const noexcept;

		bool addParameter(mref<MaterialPrototypeParameter> param_);

		[[nodiscard]] tl::optional<cref<MaterialPrototypeParameter>> getParameter(
			ParameterIdentifier identifier_
		) const noexcept;

		[[nodiscard]] tl::optional<cref<MaterialPrototypeParameter>> getFirstParameter(
			StringView name_
		) const noexcept;

		bool removeParameter(
			ParameterIdentifier identifier_
		);

		bool removeFirstParameter(StringView uniqueName_);
	};
}
