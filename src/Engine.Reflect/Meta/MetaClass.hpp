#pragma once
#include <span>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace hg {
    template <typename, typename>
    class TypedMetaClass;
}

namespace hg {
    class MetaClass {
    public:
        using this_type = MetaClass;

    protected:
        constexpr MetaClass(const type_id typeId_) noexcept :
            _typeId(typeId_) {}

    public:
        constexpr virtual ~MetaClass() noexcept = default;

    protected:
        #pragma region Meta Information
        type_id _typeId;

    public:
        [[nodiscard]] constexpr cref<type_id> typeId() const noexcept {
            return _typeId;
        }

        [[nodiscard]] constexpr virtual bool inherits(const ptr<const MetaClass> type_) const noexcept = 0;

        [[nodiscard]] constexpr virtual bool inherits(const type_id typeId_) const noexcept = 0;

        #pragma endregion

    protected:
        #pragma region Invocation Interface
        // TODO: Capture invocation interface
        #pragma endregion
    };
}
