#pragma once

#include <Engine.Common/Wrapper.hpp>
#include "Inherit/Concept.hpp"
#include "Inherit/ClassMetaBase.hpp"

namespace hg {
    /**
     * Forward Declarations
     */

    //template <typename TargetType_, typename SrcType_, typename... Params_>
    //auto Cast(auto&& src_);

    /**
     *
     */

    template <typename TargetType_, typename SrcType_, bool Secure_ = true> requires
        ClassHasMeta<TargetType_> && ClassHasMeta<SrcType_> && _STD derived_from<SrcType_, TargetType_>
    ptr<TargetType_> Cast(ptr<SrcType_> src_) noexcept {
        // We can safely up-cast straight forward
        return static_cast<ptr<TargetType_>>(src_);
    }

    template <typename TargetType_, typename SrcType_, bool Secure_ = true> requires
        ClassHasMeta<TargetType_> && ClassHasMeta<SrcType_> && (not _STD derived_from<SrcType_, TargetType_>)
    ptr<TargetType_> Cast(ptr<SrcType_> src_) noexcept {
        // We need to do some runtime checks to ensure cast hierarchy
        if constexpr (Secure_) {
            return src_->getMetaClass()->template is<TargetType_>() ?
                       static_cast<ptr<TargetType_>>(src_) :
                       nullptr;
        }
        return static_cast<ptr<TargetType_>>(src_);
    }

    template <typename TargetType_, typename SrcType_, bool Secure_ = true> requires
        ClassHasMeta<TargetType_> && ClassHasMeta<SrcType_> && (not _STD derived_from<SrcType_, TargetType_>)
    const ptr<const TargetType_> Cast(const ptr<const SrcType_> src_) noexcept {
        // We need to do some runtime checks to ensure cast hierarchy
        if constexpr (Secure_) {
            return src_->getMetaClass()->template is<TargetType_>() ?
                       static_cast<const ptr<const TargetType_>>(src_) :
                       nullptr;
        }
        return static_cast<const ptr<const TargetType_>>(src_);
    }
}
