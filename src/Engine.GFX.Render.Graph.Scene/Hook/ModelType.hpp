#pragma once

#include <Engine.Common/Math/Hash.hpp>
#include <Engine.Reflect/Meta/TypeId.hpp>

#include "../SceneHookFilter.hpp"

namespace hg::engine::render::graph {
    enum class ModelTypeFilterMode : bool {
        eWhitelist = false,
        eBlacklist = true
    };

    class ModelTypeFilter :
        public InheritMeta<ModelTypeFilter, SceneHookFilter> {
    public:
        using this_type = ModelTypeFilter;

    public:
        constexpr ModelTypeFilter() noexcept = default;

        constexpr ModelTypeFilter(
            mref<Vector<ptr<const MetaClass>>> typeMetaClasses_,
            ModelTypeFilterMode filterMode_ = ModelTypeFilterMode::eWhitelist
        ) noexcept :
            InheritMeta(),
            _filterMode(filterMode_),
            _typeMetaClasses(_STD move(typeMetaClasses_)) {}

    private:
        ModelTypeFilterMode _filterMode;
        Vector<ptr<const MetaClass>> _typeMetaClasses;

    public:
        [[nodiscard]] checksum_type checksum() const noexcept override {

            constexpr auto selfTypeId = reflect::typeId<this_type>();
            checksum_type combined { selfTypeId.data };

            for (const auto& metaType_ : _typeMetaClasses) {
                hash::hashCombine<checksum_type>(combined, metaType_->typeId().data);
            }

            return combined;
        }

    public:
        [[nodiscard]] bool operator()(const ptr<RenderSceneSystemModel> model_) const noexcept override;
    };
}
