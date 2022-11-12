#pragma once

#include <Engine.Common/Collection/BytellHashMap.hpp>
#include <Engine.Common/Meta/Concept.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

#include "SceneHierarchy/HierarchyResolver.hpp"
#include "SceneHierarchy/HierarchyGenerator.hpp"

namespace ember::editor::ui {
    class SceneHierarchyPanel;
}

namespace ember::editor::ui {

    class SceneHierarchy {
    public:
        using this_type = SceneHierarchy;

    public:
        SceneHierarchy();

        ~SceneHierarchy();

    private:
    public:
        Vector<wptr<SceneHierarchyPanel>> _panels;

    public:
        [[nodiscard]] sptr<SceneHierarchyPanel> makePanel();

    private:
        ska::bytell_hash_map<type_id, uptr<HierarchyResolverBase>> _resolver;

    public:
        bool storeResolver(const type_id typeId_, mref<uptr<HierarchyResolverBase>> resolver_);

        template <HasStaticType SourceType_, typename ResolveType_ = SourceType_>
        bool storeTypedResolver(mref<uptr<HierarchyResolver<SourceType_, ResolveType_>>> resolver_) {
            return storeResolver(SourceType_::type_id, _STD move(resolver_));
        }

        [[nodiscard]] ptr<HierarchyResolverBase> getResolver(const type_id typeId_);

        template <HasStaticType SourceType_, typename ResolveType_ = SourceType_>
        [[nodiscard]] ptr<HierarchyResolver<SourceType_, ResolveType_>> getTypedResolver() {
            return getResolver(SourceType_::type_id);
        }

    private:
        ska::bytell_hash_map<type_id, uptr<HierarchyGeneratorBase>> _generator;

    public:
        bool storeGenerator(const type_id typeId_, mref<uptr<HierarchyGeneratorBase>> resolver_);

        template <typename SourceType_> requires
            (HasStaticType<SourceType_> || HasStaticType<_STD decay_t<SourceType_>>)
        bool storeTypedGenerator(mref<uptr<HierarchyGenerator<SourceType_>>> resolver_) {
            return storeGenerator(_STD decay_t<SourceType_>::type_id, _STD move(resolver_));
        }

        [[nodiscard]] ptr<HierarchyGeneratorBase> getGenerator(const type_id typeId_);

        template <typename SourceType_> requires
            (HasStaticType<SourceType_> || HasStaticType<_STD decay_t<SourceType_>>)
        [[nodiscard]] ptr<HierarchyGenerator<SourceType_>> getTypedGenerator() {
            return getGenerator(_STD decay_t<SourceType_>::type_id);
        }
    };

}
