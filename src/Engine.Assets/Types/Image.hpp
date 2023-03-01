#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "Asset.hpp"

namespace hg::engine::assets {
    class Image final :
        public Asset {
    public:
        template <typename>
        friend class serialization::DataLayout;

        template <class>
        friend class serialization::access::Structure;

    public:
        inline static constexpr asset_type_id typeId { "Image"_typeId };

    private:
    public:
        Image();

    protected:
        Image(cref<asset_guid> guid_);

    public:
        Image(cref<asset_guid> guid_, mref<Vector<Url>> sources_);

    private:
        Vector<Url> _sources;

    public:
        [[nodiscard]] cref<Vector<Url>> sources() const noexcept;

        void addSource(cref<Url> sourceUrl_);

        void removeSource(cref<Url> sourceUrl_);
    };
}
