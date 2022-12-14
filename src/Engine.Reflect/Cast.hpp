#pragma once

#include <Engine.Common/Wrapper.hpp>
#include "EmberReflect.hpp"

namespace ember {
    /**
     * Forward Declarations
     */

    template <typename TargetType_, typename SrcType_, typename... Params_>
    auto Cast(auto&& src_);

    /**
     *
     */

    template <typename TargetType_, typename SrcType_, bool Secure_ = true> requires
        IsEmberObject<TargetType_> && IsEmberObject<SrcType_> && _STD derived_from<SrcType_, TargetType_>
    ptr<TargetType_> Cast(ptr<SrcType_> src_) noexcept {
        // We can safely up-cast straight forward
        return static_cast<ptr<TargetType_>>(src_);
    }

    template <typename TargetType_, typename SrcType_, bool Secure_ = true> requires
        IsEmberObject<TargetType_> && IsEmberObject<SrcType_> && (not _STD derived_from<SrcType_, TargetType_>)
    ptr<TargetType_> Cast(ptr<SrcType_> src_) noexcept {
        // We need to do some runtime checks to ensure cast hierarchy
        if constexpr (Secure_) {
            return src_->getClass()->template isType<TargetType_>() ?
                       static_cast<ptr<TargetType_>>(src_) :
                       nullptr;
        }
        return static_cast<ptr<TargetType_>>(src_);
    }
}
