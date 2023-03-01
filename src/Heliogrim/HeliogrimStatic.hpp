#pragma once

#include <concepts>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Make.hpp>

#include <Engine.Core/Engine.hpp>

namespace hg::engine::core {
    class Session;
}

namespace hg {
    class HeliogrimStatic {
    private:
        inline static nular_fnc<uptr<engine::Engine>> _staticInitializer = nullptr;

        template <class EngineType_>
        static uptr<engine::Engine> useEngineHelper() {
            return make_uptr<EngineType_>();
        }

    public:
        template <class EngineType_> requires
            _STD is_default_constructible_v<EngineType_> &&
            _STD derived_from<EngineType_, engine::Engine>
        static void useEngine() {
            _staticInitializer = HeliogrimStatic::useEngineHelper<EngineType_>;
        }

        [[nodiscard]] static bool hasInitializer() noexcept {
            return _staticInitializer != nullptr;
        }

        [[nodiscard]] static uptr<engine::Engine> initialize() {
            if (_staticInitializer) {
                return _staticInitializer();
            }
            return nullptr;
        }

    private:
        inline static const_bound_tuple_fnc<
            engine::Engine, const non_owning_rptr<engine::core::Session>> _staticBoundSessionGetter = nullptr;

    public:
        [[nodiscard]] static decltype(_staticBoundSessionGetter) sessionGetter() noexcept {
            return _staticBoundSessionGetter;
        }

        [[nodiscard]] static bool hasSessionGetter() noexcept {
            return _staticBoundSessionGetter != nullptr;
        }

        template <class EngineType_> requires
            _STD is_default_constructible_v<EngineType_> &&
            _STD is_convertible_v<ptr<_STD decay_t<EngineType_>>, ptr<engine::Engine>> &&
            _STD derived_from<EngineType_, engine::Engine>
        static void useSessionGetter(
            const const_bound_tuple_fnc<EngineType_, const non_owning_rptr<engine::core::Session>> fnc_
        ) {
            _staticBoundSessionGetter = reinterpret_cast<const decltype(_staticBoundSessionGetter)>(fnc_);
        }
    };
}
