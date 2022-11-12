#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace ember::engine::reflow {
    class Widget;
}

namespace ember::editor::ui {

    class HierarchyGeneratorBase {
    public:
        template <typename SourceType_>
        friend class HierarchyGenerator;

    public:
        using this_type = HierarchyGeneratorBase;

    private:
        HierarchyGeneratorBase() = default;

    public:
        virtual ~HierarchyGeneratorBase() = default;
    };

    template <typename SourceType_>
    class HierarchyGenerator :
        public HierarchyGeneratorBase {
    public:
        using this_type = HierarchyGenerator<SourceType_>;

        using source_type = SourceType_;

    public:
        HierarchyGenerator() :
            HierarchyGeneratorBase() {}

        ~HierarchyGenerator() override = default;

    public:
        sptr<engine::reflow::Widget> operator()(cref<SourceType_> source_) const;
    };

    template <>
    inline sptr<engine::reflow::Widget> HierarchyGenerator<sptr<void>>::operator()(
        cref<sptr<void>> source_
    ) const {
        return nullptr;
    }
}
