#pragma once
#include <concepts>

#include "RenderSceneSystemModel.hpp"

namespace hg::engine::render {
    struct __declspec(novtable) RenderSceneSystemSubStorageBase {
        virtual ~RenderSceneSystemSubStorageBase() noexcept = default;

        virtual const ptr<const MetaClass> getDataMetaClass() const noexcept = 0;

        virtual volatile nmpt<RenderSceneSystemModel> add(ptr<SceneComponent> src_) = 0;

        virtual void remove(ptr<ClassMetaBase> obj_) = 0;

    public:
        virtual void forEach(std::function<void(cref<RenderSceneSystemModel>)> fn_) const = 0;

        virtual void forEachMut(std::function<void(ref<RenderSceneSystemModel>)> fn_) = 0;
    };

    /**/

    template <typename Type_> requires std::derived_from<Type_, RenderSceneSystemModel>
    struct RenderSceneSystemSubStorage final :
        public RenderSceneSystemSubStorageBase {
        using this_type = RenderSceneSystemSubStorage<Type_>;

        RenderSceneSystemSubStorage() noexcept :
            RenderSceneSystemSubStorageBase(),
            dataMetaClass(Type_::getStaticMetaClass()),
            denseData() {}

        RenderSceneSystemSubStorage(this_type&& other_) noexcept :
            RenderSceneSystemSubStorageBase(),
            dataMetaClass(std::move(other_.dataMetaClass)),
            denseData(std::move(other_.denseData)) {}

        RenderSceneSystemSubStorage(const this_type&) = delete;

        ~RenderSceneSystemSubStorage() override = default;

        ref<this_type> operator=(mref<this_type> other_) = delete;

        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> swap(this_type& other_) {
            if (std::addressof(other_) == this) {
                return *this;
            }

            if (other_.dataMetaClass != dataMetaClass) {
                throw std::runtime_error("Tried to swap storage container while allocated types are different.");
            }

            std::swap(denseData, other_.denseData);
            return *this;
        }

        ptr<const MetaClass> dataMetaClass;
        Vector<Type_> denseData;

        const ptr<const MetaClass> getDataMetaClass() const noexcept override {
            return dataMetaClass;
        }

        volatile nmpt<RenderSceneSystemModel> add(ptr<SceneComponent> src_) override {
            denseData.emplace_back(std::forward<decltype(src_)>(src_));
            return &denseData.back();
        }

        void remove(ptr<ClassMetaBase> obj_) override {

            const auto diff = reinterpret_cast<uintptr_t>(obj_) - reinterpret_cast<uintptr_t>(
                std::addressof(*denseData.begin())
            );
            const auto removeIt = denseData.begin() + diff;

            denseData.erase(removeIt);
        }

    public:
        template <typename Fn_>
        void forEach(Fn_&& fn_) const {
            for (const auto& stored : denseData) {
                fn_(stored);
            }
        }

        void forEach(std::function<void(cref<RenderSceneSystemModel>)> fn_) const override {
            for (const auto& stored : denseData) {
                fn_(stored);
            }
        }

        template <typename Fn_>
        void forEachMut(Fn_&& fn_) const {
            for (auto& stored : denseData) {
                fn_(stored);
            }
        }

        void forEachMut(std::function<void(ref<RenderSceneSystemModel>)> fn_) override {
            for (auto& stored : denseData) {
                fn_(stored);
            }
        }
    };
}
