#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../Component/Component.hpp"

namespace hg::engine::gfx::render::graph {
    class Auxiliary {
    public:
        using this_type = Auxiliary;

    public:
        Auxiliary() noexcept = default;

        ~Auxiliary();

    private:
        void tidy();

    private:
        CompactSet<ptr<Component>> _comps;

    public:
        [[nodiscard]] cref<CompactSet<ptr<Component>>> components() const noexcept;

        /**
         * Add a component to the auxiliary set
         *
         * @param component_ A unique pointer tot the component to store
         *
         * @details This will unwrap the unique pointer internally and rely on
         *  deletion after remove or on auxiliary destruction.
         */
        void add(mref<uptr<Component>> component_) noexcept;

        /**
         * Remove component from auxiliary set
         *
         * @param component_ A pointer to the component to remove
         *
         * @warning This will implicitly erase/delete the given component.
         */
        void remove(_In_ mref<ptr<Component>> component_) noexcept;
    };
}
