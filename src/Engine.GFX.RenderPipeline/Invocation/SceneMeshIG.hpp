#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Scene/GeometryModel.hpp>
#include <Engine.GFX/Scene/SkeletalGeometryModel.hpp>
#include <Engine.GFX/Scene/StaticGeometryModel.hpp>

#include "ComplexInvocationGenerator.hpp"
#include "InvocationGenerator.hpp"
#include "ProcMeshIG.hpp"
#include "SceneWalker.hpp"
#include "StaticMeshIG.hpp"
#include "../__fwd.hpp"

namespace hg::engine::gfx::render::pipeline {
    template <typename... StaticIgTypes_>
    class __declspec(novtable) SceneMeshBaseIG :
        public ComplexInvocationGenerator {
    protected:
        constexpr SceneMeshBaseIG() noexcept = default;

        constexpr ~SceneMeshBaseIG() noexcept override = default;

    private:
        _STD tuple<StaticIgTypes_...> _staticIgs;
        Vector<uptr<InvocationGenerator>> _dynamicIgs;

    protected:
        template <typename StaticIgType_>
        [[nodiscard]] const auto& getStaticIg() const noexcept {
            return _STD get<StaticIgType_>(_staticIgs);
        }

        template <typename StaticIgType_>
        [[nodiscard]] auto& getStaticIg() noexcept {
            return _STD get<StaticIgType_>(_staticIgs);
        }

        template <typename Fn_> requires _STD is_invocable_v<Fn_, ref<InvocationGenerator>>
        void forEachStaticIg(Fn_&& fn_) noexcept {
            ((fn_(_STD get<StaticIgTypes_>(_staticIgs))), ...);
        }

        [[nodiscard]] cref<Vector<uptr<InvocationGenerator>>> getDynamicIgs() const noexcept {
            return _dynamicIgs;
        }

        void addDynamicIg(mref<uptr<InvocationGenerator>> ig_) {
            _dynamicIgs.emplace_back(_STD move(ig_));
        }

        void removeDynamicIg(mref<ptr<InvocationGenerator>> ig_) {
            _STD erase_if(
                _dynamicIgs,
                [ig_](cref<uptr<InvocationGenerator>> entry_) {
                    return entry_.get() == ig_;
                }
            );
        }
    };

    class SceneMeshIG :
        public SceneMeshBaseIG<StaticMeshIG, ProcMeshIG> {
    public:
        SceneMeshIG() noexcept;

        SceneMeshIG(mref<smr<SceneWalker>> sceneWalker_) noexcept;

        ~SceneMeshIG() override;

    private:
        smr<SceneWalker> _sceneWalker;

    private:
        [[nodiscard]] nmpt<IGCommandBuffer> getIgCmdBuffer();

    public:
        void accept(mref<nmpt<const GeometryModel>> model_) {

            if (model_->getMetaClass()->exact<StaticGeometryModel>()) {
                // nmpt<const StaticGeometryModel> model = _STD move(model_);

            } else if (model_->getMetaClass()->exact<SkeletalGeometryModel>()) {
                // nmpt<const SkeletalGeometryModel> model = _STD move(model_);
            }

            // TODO:
            model_->render(getIgCmdBuffer());
        }

    protected:
        [[nodiscard]] ref<StaticMeshIG> getStaticMeshIg() noexcept;

        [[nodiscard]] ref<ProcMeshIG> getProcMeshIg() noexcept;

        [[nodiscard]] ref<InvocationGenerator> selectSubGenerator(type_id modelTypeId_) const noexcept;

    public:
        void operator()();

        void operator()(const ptr<const SceneWalker::scene_model_type> model_);

        [[nodiscard]] IGProcessResult operator()(
            mref<smr<const acc::AccelerationPass>> acceleration_,
            mref<uptr<IGCommandBuffer>> igcb_
        ) override;

        tl::expected<AccelerationCommandBuffer, IGError> finalize() noexcept override;
    };
}
