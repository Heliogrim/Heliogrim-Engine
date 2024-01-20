#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/Collection/Set.hpp>

#include "../Task/Task.hpp"

#include "Stage/Stage.hpp"
#include "Stage/StageRegister.hpp"
#include "Composite/StageDependency.hpp"

namespace hg::engine::scheduler {
    class __declspec(novtable) StagePipeline {
    public:
        using this_type = StagePipeline;

        using identifier_type = AssocKey<string>;
        using task_type = ::hg::engine::scheduler::task::TaskDelegate;

    protected:
        StagePipeline(mref<identifier_type> identifier_) noexcept;

    public:
        virtual ~StagePipeline() noexcept;

    protected:
        /**
         * Unique Identifier
         */
        const identifier_type _identifier;

    public:
        [[nodiscard]] cref<identifier_type> identifier() const noexcept;

    private:
        /**
         * Stage mounting and dismounting
         */

    public:
        virtual void mount(const non_owning_rptr<StageRegister> register_) = 0;

        virtual void declareDependencies(
            const non_owning_rptr<const StageRegister> register_,
            _Inout_ ref<CompactSet<StageDependency>> collection_
        ) = 0;

        virtual void dismount(const non_owning_rptr<StageRegister> register_) = 0;

    protected:
        /**
         * Mounted stages cached for working and queries
         */
        Vector<non_owning_rptr<const Stage>> _orderedStages;

    public:
        [[nodiscard]] const non_owning_rptr<const Stage> beginStage() const noexcept;

        [[nodiscard]] cref<Vector<non_owning_rptr<const Stage>>> orderedStages() const noexcept;

        [[nodiscard]] const non_owning_rptr<const Stage> endStage() const noexcept;

    public:
        /**
         * Task helper functions
         */
        [[nodiscard]] bool isSharedTask(const non_owning_rptr<const task_type> task_) const noexcept;

        [[nodiscard]] bool isExclusiveTask(const non_owning_rptr<const task_type> task_) const noexcept;

    public:
        /**
         * Attributes / Behaviour of this pipeline
         */
        [[nodiscard]] virtual bool isSkippable() const noexcept = 0;
    };
}
