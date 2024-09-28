#pragma once
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg::engine::render::graph {
	enum class DescriptionValueMatchingMode : u8 {
		eIgnored = 0b0000'0000,
		eCovariant = 0b0000'0001,
		eInvariant = 0b0000'0011
	};

	template <typename Type_, typename ModeType_ = DescriptionValueMatchingMode>
	struct DescriptionValue {
		using this_type = DescriptionValue<Type_, ModeType_>;
		using mode_type = ModeType_;
		using value_type = Type_;

		mode_type mode;
		value_type value;
	};

	template <typename Type_>
	DescriptionValue(DescriptionValueMatchingMode, Type_) -> DescriptionValue<Type_, DescriptionValueMatchingMode>;

	class macro_novtable Description :
		public InheritBase<Description> {
	public:
		using this_type = Description;

	protected:
		constexpr Description() noexcept = default;

	public:
		Description(mref<this_type> other_) noexcept = default;

		Description(cref<this_type> other_) noexcept = default;

		~Description() override = default;

	protected:
		[[nodiscard]] virtual bool isValueCompatible(
			const non_owning_rptr<const Description> other_
		) const noexcept = 0;

	public:
		[[nodiscard]] bool isCompatible(const non_owning_rptr<const Description> other_) const noexcept;
	};
}
