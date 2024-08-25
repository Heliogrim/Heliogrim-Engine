#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/Type.hpp>
#include "Inherit/ClassMetaBase.hpp"
#include "Inherit/Concept.hpp"

namespace hg {
	/**/

	template <CompleteType TargetType_, CompleteType SrcType_, bool Secure_ = true> requires
		ClassHasMeta<TargetType_> && ClassHasMeta<SrcType_> && std::derived_from<SrcType_, TargetType_>
	ptr<TargetType_> Cast(ptr<SrcType_> src_) noexcept {
		// We can safely up-cast straight forward
		return static_cast<ptr<TargetType_>>(src_);
	}

	template <CompleteType TargetType_, CompleteType SrcType_, bool Secure_ = true> requires
		ClassHasMeta<TargetType_> && ClassHasMeta<SrcType_> && (not std::derived_from<SrcType_, TargetType_>)
	ptr<TargetType_> Cast(ptr<SrcType_> src_) noexcept {
		// We need to do some runtime checks to ensure cast hierarchy
		if constexpr (Secure_) {
			return src_->getMetaClass()->template is<TargetType_>() ?
				static_cast<ptr<TargetType_>>(src_) :
				nullptr;
		}
		return static_cast<ptr<TargetType_>>(src_);
	}

	template <CompleteType TargetType_, CompleteType SrcType_, bool Secure_ = true> requires
		ClassHasMeta<TargetType_> && ClassHasMeta<SrcType_> && (not std::derived_from<SrcType_, TargetType_>)
	ptr<const TargetType_> Cast(const ptr<const SrcType_> src_) noexcept {
		// We need to do some runtime checks to ensure cast hierarchy
		if constexpr (Secure_) {
			return src_->getMetaClass()->template is<TargetType_>() ?
				static_cast<const ptr<const TargetType_>>(src_) :
				nullptr;
		}
		return static_cast<const ptr<const TargetType_>>(src_);
	}
}

/**/

#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg {
	template <
		CompleteType TargetType_,
		CompleteType SrcType_,
		bool Secure_ = true
	> requires
		ClassHasMeta<TargetType_> &&
		ClassHasMeta<typename SrcType_::value_type> &&
		(not std::derived_from<typename SrcType_::value_type, TargetType_>)
	[[nodiscard]] nmpt<std::conditional_t<
		std::is_const_v<typename SrcType_::value_type>, const TargetType_, TargetType_
	>> Cast(
		const SrcType_ src_
	) noexcept {
		using target_type = std::conditional_t<
			std::is_const_v<typename SrcType_::value_type>,
			const TargetType_,
			TargetType_
		>;

		if constexpr (Secure_) {
			return src_->getMetaClass()->template is<TargetType_>() ?
				nmpt<target_type> { static_cast<const ptr<target_type>>(src_.get()) } :
				nmpt<target_type> { nullptr };
		}
		return nmpt<target_type> { static_cast<const ptr<target_type>>(src_.get()) };
	}
}
