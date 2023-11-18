#pragma once
#include <utility>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx::render {
    class __declspec(novtable) MeshCaptureInterface {
    public:
        using this_type = MeshCaptureInterface;

    public:
        constexpr MeshCaptureInterface() noexcept = default;

        constexpr virtual ~MeshCaptureInterface() noexcept = default;

    public:
        // TODO: Virtual base for dynamic resolved

        template <typename CrtpType_, typename Type_>
        struct DynamicResolved {
            [[nodiscard]] constexpr CrtpType_& self() noexcept {
                return *this;
            }

            [[nodiscard]] auto operator()(auto&&... args_) noexcept {
                return self()(_STD forward<decltype(args_)>(args_)...);
            }
        };

        template <typename Type_>
        struct ForbiddenDynamicResolved :
            public DynamicResolved<ForbiddenDynamicResolved<Type_>, Type_> {
            [[nodiscard]] auto operator()(auto&&...) noexcept {
                _STD unreachable();
            }
        };

    public:
        [[nodiscard]] ptr<DynamicResolved<void, void>> defer(void*) noexcept;

        template <typename Type_>
        [[nodiscard]] ptr<ForbiddenDynamicResolved<Type_>> defer(auto&&... args_) noexcept {
            return ptr<ForbiddenDynamicResolved<Type_>> { defer(nullptr) };
        }

    public:
        virtual void store(mref<struct MeshCaptureBatch>) noexcept = 0;
    };
}
