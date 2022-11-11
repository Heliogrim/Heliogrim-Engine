#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflow/Widget/Box.hpp>

namespace ember::editor::ui {

    class ObjectValueMapperBase {
    public:
        template <typename ObjectType_>
        friend class ObjectValueMapper;

    public:
        using this_type = ObjectValueMapperBase;

    private:
        ObjectValueMapperBase() = default;

    public:
        virtual ~ObjectValueMapperBase() = default;

    public:
        virtual void update(const ptr<void> obj_) = 0;

        virtual void build(cref<sptr<engine::reflow::Box>> parent_) = 0;

        virtual void cleanup(cref<sptr<engine::reflow::Box>> parent_) = 0;
    };

    template <typename ObjectType_>
    class ObjectValueMapper :
        public ObjectValueMapperBase {
    public:
        using this_type = ObjectValueMapper<ObjectType_>;

    public:
        ObjectValueMapper() :
            ObjectValueMapperBase() {}

        ~ObjectValueMapper() override = default;

    public:
        void update(const ptr<void> obj_) override;

        void build(cref<sptr<engine::reflow::Box>> parent_) override;

        void cleanup(cref<sptr<engine::reflow::Box>> parent_) override;
    };

    template <>
    inline void ObjectValueMapper<void>::update(const ptr<void> obj_) {}

    template <>
    inline void ObjectValueMapper<void>::build(cref<sptr<engine::reflow::Box>> parent_) {}

    template <>
    inline void ObjectValueMapper<void>::cleanup(cref<sptr<engine::reflow::Box>> parent_) {}

}
