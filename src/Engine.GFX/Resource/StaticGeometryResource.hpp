#pragma once
#include <Engine.Resource/Manage/Resource.hpp>

#include "../Buffer/Buffer.hpp"

namespace ember::engine::gfx {

    class StaticGeometryResource :
        public res::Resource {
    public:
        using this_type = StaticGeometryResource;
        using underlying_type = res::Resource;

    public:
        StaticGeometryResource() = default;

        ~StaticGeometryResource() override = default;

        //private:
    public:
        struct VertexData {
            VertexBuffer buffer;
        } _vertexData;

        struct IndexData {
            IndexBuffer buffer;
        } _indexData;

    public:
        [[nodiscard]] loaded_flag_type loaded() const noexcept override {
            return {};
        }

        [[nodiscard]] res::ManageGuard<Resource> acquire(const res::ResourceUsageFlags flags_) override {
            return {};
        }

        [[nodiscard]] bool try_acquire(ptr<res::ManageGuard<Resource>> guard_,
            const res::ResourceUsageFlags flags_) noexcept override {
            return false;
        }

        void release(const res::ResourceUsageFlags flags_) override {}
    };
}
