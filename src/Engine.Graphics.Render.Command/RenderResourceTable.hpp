#pragma once
#include <Engine.Common/Collection/Vector.hpp>

#include "RenderResource.hpp"

namespace hg::engine::render {
    class RenderResourceTable {
    public:
        using this_type = RenderResourceTable;

    public:
        Vector<cmd::RCmdRef<void>> resources;

    public:
        /*
        template <typename ResourceType_, template <typename...> typename AccessType_>
        [[nodiscard]] cmd::RCmdRef<AccessType_<ResourceType_>> allocate() {

            auto val = make_smr<AccessType_<ResourceType_>>();
            resources.push_back(val);

            return val;
        }

        template <
            typename ResourceType_,
            template <typename...> typename AccessType_
        >
        [[nodiscard]] cmd::RCmdRef<AccessType_<ResourceType_>> allocate(mref<smr<const ResourceType_>> resource_) {

            auto val = make_smr<AccessType_<ResourceType_>>(std::move(resource_));
            resources.push_back(val);

            return val;
        }
         */

        template <typename ResourceType_, template <typename...> typename AccessType_ = std::type_identity_t>
        [[nodiscard]] cmd::RCmdRef<ResourceType_> allocate() {

            auto val = make_smr<AccessType_<ResourceType_>>();
            resources.push_back(val);

            return val;
        }
    };
}
