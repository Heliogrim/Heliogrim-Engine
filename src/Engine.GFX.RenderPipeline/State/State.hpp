#pragma once

#include <Engine.Common/Buffer.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Cache/LocalCacheCtrl.hpp>
#include <Engine.GFX/Memory/LocalPooledAllocator.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

#include "../__fwd.hpp"
#include "SubStateInfo.hpp"

namespace hg::engine::gfx::render::pipeline {
    class State {
    public:
        using this_type = State;

    public:
        State(
            mref<uptr<cache::LocalResourceCache>> cache_,
            mref<memory::LocalPooledAllocator> allocator_
        ) noexcept;

        State(mref<State>) noexcept = delete;

        State(cref<State>) = delete;

        ~State() noexcept = default;

    private:
        Vector<nmpt<const Stage>> _path;

    public:
        [[nodiscard]] cref<Vector<nmpt<const Stage>>> getPath() const noexcept;

    private:
        /**
         * Resource Management
         */

        cache::LocalCacheCtrl _cacheCtrl;
        memory::LocalPooledAllocator _gfxAllocator;

    public:
        [[nodiscard]] ref<cache::LocalCacheCtrl> cache() noexcept;

        [[nodiscard]] ref<memory::LocalPooledAllocator> gfxAllocator() noexcept;

    private:
        /**
         * State Memory Cluster
         */
        // TODO: Replace with arena allocator and arena memory block
        Vector<SubStateInfo> _regSubState;
        hg::Buffer _arena;

        // Used for dynamic allocations without pre-registered types
        Vector<void*> _memList;

    private:
        void regSubState(type_id type_, size_t byteSize_, size_t key_ = ~0ui64);

        [[nodiscard]] void* allocSubState(type_id type_, size_t key_ = ~0ui64);

        [[nodiscard]] void* allocSubState(size_t byteSize_);

        void freeSubState(_In_ mref<nmpt<void>> ptr_);

    public:
        /**
         * Will store meta infos about the registered type
         *
         * @details The target type will be resolved by the reflected static identifier.
         *  Additionally the captured info will be used to pre-layout the arena memory;
         *
         * @warning Registering types without additional key values will limit
         *      the possible stored instances to one instance per type.
         */
        template <SubStateType Type_>
        void regSubState() {

            const auto type = HeliogrimClass::stid<Type_>();
            const auto size = sizeof(Type_);

            regSubState(type, size);
        }

        /**
         * Will store meta infos about the registered type
         *
         * @details The target type will be resolved by the reflected static identifier.
         *  Additionally the captured info will be used to pre-layout the arena memory.
         *
         * @param key_ The additional key to identify the target info and instance.
         *
         * @details The key_ value ~0ui64 is a reserved identifier.
         *
         */
        template <SubStateType Type_>
        void regSubState(size_t key_) {

            const auto type = HeliogrimClass::stid<Type_>();
            const auto size = sizeof(Type_);

            regSubState(type, size, key_);
        }

        template <SubStateType Type_>
        [[nodiscard]] nmpt<Type_> allocSubState() {

            void* mem = nullptr;
            if constexpr (HasStaticType<Type_>) {

                constexpr auto type = HeliogrimClass::stid<Type_>();
                mem = allocSubState(type);

            }

            if (mem == nullptr) {
                const auto size = sizeof(Type_);
                mem = allocSubState(size);

                #ifdef _DEBUG
                if constexpr (HasStaticType<Type_>) {
                    IM_CORE_WARN(
                        "Encountered statically typed sub-state for dynamic allocation. Consider pre-registering the state for better memory cohesion."
                    );
                }
                #endif
            }

            return new(mem) Type_();
        }

        /**
         * Will allocate the desired memory if a meta info is presented.
         *
         * @tparam Type_ The type used to identify and construct the state.
         * @param key_ The key_ value used to differentiate the meta infos.
         *
         * @details The key_ value ~0ui64 is a reserved identifier.
         * 
         */
        template <SubStateType Type_> requires HasStaticType<Type_>
        [[nodiscard]] nmpt<Type_> allocSubState(size_t key_) {

            constexpr auto type = HeliogrimClass::stid<Type_>();
            auto* mem = allocSubState(type, key_);
            assert(mem != nullptr);

            return new(mem) Type_();
        }

        template <SubStateType Type_>
        void deallocSubState(mref<nmpt<Type_>> ptr_) {
            ptr_->~Type_();
            freeSubState(_STD move(ptr_));
        }
    };
}
