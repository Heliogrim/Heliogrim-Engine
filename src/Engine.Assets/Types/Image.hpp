#pragma once

#include <Engine.Filesystem/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "Asset.hpp"

namespace hg::engine::assets {
    class Image final :
        public InheritMeta<Image, Asset> {
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
        Image(mref<asset_guid> guid_);

    public:
        Image(mref<asset_guid> guid_, mref<Vector<fs::Url>> sources_);

    private:
        Vector<fs::Url> _sources;

    public:
        [[nodiscard]] cref<Vector<fs::Url>> sources() const noexcept;

        void addSource(cref<fs::Url> sourceUrl_);

        void removeSource(cref<fs::Url> sourceUrl_);
    };
}
