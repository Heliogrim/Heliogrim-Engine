#pragma once

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

    struct __declspec(novtable) Annotation {
    public:
        using this_type = Annotation;

    public:
        constexpr Annotation(const AnnotationType type_, mref<uptr<Annotation>> next_) noexcept :
            type(type_),
            next(_STD move(next_)) {}

        Annotation(cref<this_type> other_) :
            type(other_.type),
            next(nullptr) {
            if (other_.next != nullptr) {
                next = uptr<Annotation>(other_.next->createCopy());
            }
        }

        virtual ~Annotation() = default;

    public:
        AnnotationType type;
        uptr<Annotation> next;

        [[nodiscard]] virtual ptr<Annotation> createCopy() const = 0;
    };

    template <AnnotationType Type_>
    struct SimpleAnnotation final : public Annotation {
        constexpr SimpleAnnotation(mref<uptr<Annotation>> next_ = nullptr) noexcept :
            Annotation(Type_, _STD move(next_)) {}

        constexpr ~SimpleAnnotation() noexcept = default;

        [[nodiscard]] ptr<SimpleAnnotation<Type_>> createCopy() const final {
            return new SimpleAnnotation<Type_>(*this);
        }
    };

    template <typename Derived_>
    struct __declspec(novtable) AnnotationBase : public Annotation {
        constexpr AnnotationBase(AnnotationType type_, mref<uptr<Annotation>> next_) noexcept:
            Annotation(type_, _STD move(next_)) {}

        [[nodiscard]] ptr<Annotation> createCopy() const final {
            static_assert(_STD derived_from<Derived_, AnnotationBase>);
            return new Derived_(*static_cast<const ptr<const Derived_>>(this));
        }
    };

    struct SymbolIdAnnotation final : public AnnotationBase<SymbolIdAnnotation> {
        constexpr SymbolIdAnnotation(mref<string> symbolId_, mref<uptr<Annotation>> next_ = nullptr) noexcept:
            AnnotationBase(AnnotationType::eSymbolId, _STD move(next_)),
            symbolId(_STD move(symbolId_)) {}

        ~SymbolIdAnnotation() override = default;

        string symbolId;
    };

    struct VkBindLocationAnnotation final : public AnnotationBase<VkBindLocationAnnotation> {
        constexpr VkBindLocationAnnotation(
            const s32 vkSet_,
            const s32 vkLocation_,
            const s32 vkOffset_ = -1L,
            mref<uptr<Annotation>> next_ = nullptr
        ) noexcept :
            AnnotationBase(AnnotationType::eVkBindLocation, _STD move(next_)),
            vkSet(vkSet_),
            vkLocation(vkLocation_),
            vkOffset(vkOffset_) {}

        ~VkBindLocationAnnotation() override = default;

        s32 vkSet;
        s32 vkLocation;
        s32 vkOffset;
    };
}
