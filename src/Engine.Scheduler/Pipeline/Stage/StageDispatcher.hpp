#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Cast.hpp>

#include "../../Task/Task.hpp"

namespace hg::engine::scheduler {
    struct StageDispatcher {
    public:
        template <class SelfType_>
        using fnc_type = bound_tuple_fnc<SelfType_, void, mref<non_owning_rptr<const task::TaskDelegate>>>;

        using dispatch_fnc_type = fnc_type<StageDispatcher>;

    public:
        constexpr StageDispatcher() :
            _self(nullptr),
            _fnc(nullptr) {}

        constexpr StageDispatcher(const ptr<void> self_, dispatch_fnc_type fnc_) :
            _self(self_),
            _fnc(fnc_) {}

        template <class SelfType_>
        StageDispatcher(const ptr<SelfType_> self_, fnc_type<SelfType_> fnc_) :
            _self(self_),
            _fnc(void_cast<fnc_type<SelfType_>, dispatch_fnc_type> {}(fnc_)) {}

    private:
        ptr<void> _self;
        dispatch_fnc_type _fnc;

    public:
        FORCE_INLINE void enqueue(_In_ mref<non_owning_rptr<const task::TaskDelegate>> task_) const {
            (void)(static_cast<ptr<StageDispatcher>>(_self)->*_fnc)(
                std::forward<non_owning_rptr<const task::TaskDelegate>>(task_)
            );
        }
    };
}
