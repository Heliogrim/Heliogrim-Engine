#pragma once

#include <utility>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::accel::lang {
	enum class AnnotationType : u8 {
		eUnknown = 0x0,
		/**/
		eReadonly,
		eWriteonly,
		eUniform,
		eStorage,
		eConstant,
		/**/
		eForwardLinkage,
		eExternalLinkage,
		/**/
		eSymbolId,
		/**/
		eVkBindLocation
	};

	struct macro_novtable Annotation {
	public:
		using this_type = Annotation;

	public:
		constexpr Annotation(const AnnotationType type_, mref<uptr<Annotation>> next_) noexcept :
			type(type_),
			next(std::move(next_)) {}

		Annotation(cref<this_type> other_) :
			type(other_.type),
			next(nullptr) {
			if (other_.next != nullptr) {
				next = uptr<Annotation>(other_.next->createCopy());
			}
		}

		constexpr Annotation(mref<this_type>) noexcept = default;

		constexpr virtual ~Annotation() = default;

	public:
		AnnotationType type;
		uptr<Annotation> next;

		[[nodiscard]] virtual ptr<Annotation> createCopy() const = 0;
	};

	template <AnnotationType Type_>
	struct SimpleAnnotation final : public Annotation {
		using this_type = SimpleAnnotation<Type_>;

		constexpr SimpleAnnotation(mref<uptr<Annotation>> next_ = nullptr) noexcept :
			Annotation(Type_, std::move(next_)) {}

		SimpleAnnotation(cref<this_type> other_) :
			Annotation(other_) {}

		constexpr SimpleAnnotation(mref<this_type> other_) noexcept :
			Annotation(std::move(other_)) {}

		constexpr ~SimpleAnnotation() noexcept override = default;

		[[nodiscard]] ptr<SimpleAnnotation<Type_>> createCopy() const final {
			return new SimpleAnnotation<Type_>(*this);
		}
	};

	template <typename Derived_>
	struct macro_novtable AnnotationBase : public Annotation {
		using this_type = AnnotationBase<Derived_>;

		constexpr AnnotationBase(AnnotationType type_, mref<uptr<Annotation>> next_) noexcept:
			Annotation(type_, std::move(next_)) {}

		AnnotationBase(cref<this_type> other_) :
			Annotation(other_) {}

		constexpr AnnotationBase(mref<this_type> other_) noexcept :
			Annotation(std::move(other_)) {}

		[[nodiscard]] ptr<Annotation> createCopy() const final {
			static_assert(std::derived_from<Derived_, AnnotationBase>);
			return new Derived_(*static_cast<const ptr<const Derived_>>(this));
		}
	};

	struct SymbolIdAnnotation final : public AnnotationBase<SymbolIdAnnotation> {
		using this_type = SymbolIdAnnotation;

		constexpr SymbolIdAnnotation(mref<string> symbolId_, mref<uptr<Annotation>> next_ = nullptr) noexcept:
			AnnotationBase(AnnotationType::eSymbolId, std::move(next_)),
			symbolId(std::move(symbolId_)) {}

		SymbolIdAnnotation(cref<this_type> other_) :
			AnnotationBase(other_),
			symbolId(other_.symbolId) {}

		~SymbolIdAnnotation() override = default;

		string symbolId;
	};

	struct VkBindLocationAnnotation final : public AnnotationBase<VkBindLocationAnnotation> {
		using this_type = VkBindLocationAnnotation;

		constexpr VkBindLocationAnnotation(
			const s32 vkSet_,
			const s32 vkLocation_,
			const s32 vkOffset_ = -1L,
			mref<uptr<Annotation>> next_ = nullptr
		) noexcept :
			AnnotationBase(AnnotationType::eVkBindLocation, std::move(next_)),
			vkSet(vkSet_),
			vkLocation(vkLocation_),
			vkOffset(vkOffset_) {}

		VkBindLocationAnnotation(cref<this_type> other_) :
			AnnotationBase(other_),
			vkSet(other_.vkSet),
			vkLocation(other_.vkLocation),
			vkOffset(other_.vkOffset) {}

		constexpr VkBindLocationAnnotation(mref<this_type> other_) noexcept :
			AnnotationBase(std::move(other_)),
			vkSet(std::exchange(other_.vkSet, ~0)),
			vkLocation(std::exchange(other_.vkLocation, ~0)),
			vkOffset(std::exchange(other_.vkOffset, ~0)) {}

		~VkBindLocationAnnotation() override = default;

		s32 vkSet;
		s32 vkLocation;
		s32 vkOffset;
	};
}
