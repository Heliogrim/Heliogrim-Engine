#pragma once
#include <span>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace hg {
    template <typename TargetType_>
    class TypedMetaClass;
}

namespace hg {
    class MetaClass {
    public:
        using this_type = MetaClass;

    protected:
        MetaClass() noexcept = default;

    public:
        ~MetaClass() noexcept = default;

    protected:
        #pragma region Meta Information
        type_id _typeId;

        struct MetaClassInherit {
            DenseSet<type_id> keys {};
            Vector<ptr<const MetaClass>> details {};
        } _inherit;

    public:
        [[nodiscard]] cref<type_id> typeId() const noexcept {
            return _typeId;
        }

        [[nodiscard]] _STD span<const ptr<const MetaClass>> inherit() const noexcept {
            return _STD span {
                _inherit.details.begin(), _inherit.details.end()
            };
        }

        [[nodiscard]] _STD span<const type_id> inheritKeys() const noexcept {
            return _STD span { _inherit.keys.begin(), _inherit.keys.end() };
        }

        [[nodiscard]] bool inherits(const ptr<const MetaClass> type_) const noexcept {
            if (_STD ranges::contains(_inherit.details, type_)) {
                return true;
            }
            return _STD ranges::any_of(
                _inherit.details,
                [type_](const auto& val_) {
                    return val_->inherits(type_);
                }
            );
        }

        [[nodiscard]] bool inherits(const type_id typeId_) const noexcept {
            if (_inherit.keys.contains(typeId_)) {
                return true;
            }
            return _STD ranges::any_of(
                _inherit.details,
                [typeId_](const auto& val_) {
                    return val_->inherits(typeId_);
                }
            );
        }

        #pragma endregion

    protected:
        #pragma region Invocation Interface
        // TODO: Capture invocation interface
        #pragma endregion
    };
}
