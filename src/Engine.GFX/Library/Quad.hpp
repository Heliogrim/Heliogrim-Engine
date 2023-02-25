#pragma once

#include <Engine.Common/Make.hpp>
#include "Model.hpp"
#include "Uniform/UniformVertex.hpp"
#include "Uniform/UniformIndex.hpp"

namespace clockwork::__internal::gfx::types {
    class Quad {
    public:
        Quad();

        Quad(const __Device& device_) {
            const std::vector<Vertex> vertices = std::vector<Vertex> {
                Vertex(
                    __internal::types::vec3(1.F, 1.F, 0.F),
                    __internal::types::vec3(1.F),
                    __internal::types::vec2(1.F, 1.F),
                    __internal::types::vec3(0.F)
                ),
                Vertex(
                    __internal::types::vec3(0.F, 1.F, 0.F),
                    __internal::types::vec3(1.F),
                    __internal::types::vec2(0.F, 1.F),
                    __internal::types::vec3(0.F)
                ),
                Vertex(
                    __internal::types::vec3(0.F, 0.F, 0.F),
                    __internal::types::vec3(1.F),
                    __internal::types::vec2(0.F, 0.F),
                    __internal::types::vec3(0.F)
                ),
                Vertex(
                    __internal::types::vec3(1.F, 0.F, 0.F),
                    __internal::types::vec3(1.F),
                    __internal::types::vec2(1.F, 0.F),
                    __internal::types::vec3(0.F)
                )
            };

            const std::vector<uint32_t> indices = std::vector<uint32_t> {
                0,
                1,
                2,
                2,
                3,
                0
            };

            _model = __internal::types::shared<Model>();

            _model->vertices = device_->createBuffer(
                vk::BufferUsageFlagBits::eVertexBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                vertices.size() * sizeof(UniformVertex),
                vertices.data()
            );
            _model->indices = device_->createBuffer(
                vk::BufferUsageFlagBits::eIndexBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                indices.size() * sizeof(UniformIndex),
                indices.data()
            );

            _model->vertices.bind();
            _model->indices.bind();

            _model->indexCount = indices.size();
            _model->vertexCount = vertices.size();

            _model->lods.resize(1);

            _model->lods[0].indexBase = 0;
            _model->lods[0].indexCount = _model->indexCount;
            _model->lods[0].vertexBase = 0;
            _model->lods[0].vertexCount = _model->vertexCount;
            _model->lods[0].distance = std::numeric_limits<float>::max();

            _model->lods[0].parts.resize(1);

            _model->lods[0].parts[0].indexBase = 0;
            _model->lods[0].parts[0].indexCount = indices.size();
            _model->lods[0].parts[0].vertexBase = 0;
            _model->lods[0].parts[0].vertexCount = vertices.size();
        }

        /**
         * Returns the underlying model of the quad
         * 
         * @return Model instance wrapper as shared pointer
         */
        __Model model() const {
            return _model;
        }

    private:
        __Model _model;
    };

    U_TYPE_SHARED(Quad);
}
