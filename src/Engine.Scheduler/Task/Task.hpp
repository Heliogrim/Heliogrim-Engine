#pragma once

#include <functional>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

#include "TaskMask.hpp"
#include "TaskType.hpp"
#include "../Stage/ScheduleStage.hpp"

namespace ember::engine::scheduler::fiber {
    struct Fiber;
}

namespace ember::engine::scheduler::task {

    /**
     * ! Important !
     *
     * A Task of any [type] with any [mask] should be allocated on a dense head block of tasks to improve cache delay
     * A Task of any [type] should be based on internal type [TaskDelegate]
     * A Task will be invoked directly without type check of via [TaskDelegate] and get downcast internal
     */

    class TaskDelegate {
    public:
        /**
         * Will downcast to masked type and execute via static cast
         *
         * @author Julius
         * @date 16.11.2020
         */
        void delegate() const;

    protected:
        /**
         * Type definition of the task
         */

        /** The type */
        const TaskType _type;

        /** The mask */
        const TaskMask _mask;

    public:
        /**
         * Gets the type
         *
         * @author Julius
         * @date 13.11.2020
         *
         * @returns A TaskType.
         */
        [[nodiscard]] TaskType type() const noexcept;

        /**
         * Gets the mask
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @returns A TaskMask.
         */
        [[nodiscard]] TaskMask mask() const noexcept;

    protected:
        /**
         * Execution state of the task
         */

        /** The fiber */
        ptr<fiber::Fiber> _fiber;

    public:
        /**
         * Gets the fiber
         *
         * @author Julius
         * @date 22.10.2021
         *
         * @returns A ptr&lt;fiber::Fiber&gt;
         */
        [[nodiscard]] ptr<fiber::Fiber> fiber() const noexcept;

        /**
         * Gets the fiber
         *
         * @author Julius
         * @date 22.10.2021
         *
         * @returns A ref&lt;ptr&lt;fiber::Fiber&gt;&gt;
         */
        [[nodiscard]] ref<ptr<fiber::Fiber>> fiber() noexcept;

    protected:
        /**
         * Execution order and timing
         */

        /** The schedule stage barrier, describing the start point for this task */
        const ScheduleStageBarrier _srcStage;
        /** The schedule stage barrier, describing the latest completion for this task */
        const ScheduleStageBarrier _dstStage;
        _STD uint_fast16_t _dstBarrierIdx;

    public:
        /**
         * Get the source schedule stage
         *
         * @author Julius
         * @date 11.11.2021
         *
         * @returns The schedule stage barrier, used as source.
         */
        [[nodiscard]] ScheduleStageBarrier srcStage() const noexcept;

        /**
         * Get the destination schedule stage
         *
         * @author Julius
         * @date 11.11.2021
         *
         * @returns The schedule stage barrier, used as destination.
         */
        [[nodiscard]] ScheduleStageBarrier dstStage() const noexcept;

        /**
         * Get the index of the destination barrier
         *
         * @author Julius
         * @date 18.11.2021
         *
         * @returns The stored destination barrier index.
         */
        [[nodiscard]] _STD uint_fast16_t dstBarrierIdx() const noexcept;

        /**
         * Get the index of the destination barrier
         *
         * @author Julius
         * @date 18.11.2021
         *
         * @returns A reference to the stored destination barrier index.
         */
        [[nodiscard]] ref<_STD uint_fast16_t> dstBarrierIdx() noexcept;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param  type_ The type.
         * @param  mask_ The mask.
         * @param srcStage_ The schedule stage barrier, when to execute the task.
         * @param dstStage_ The schedule stage barrier, when the task should be completed.
         */
        TaskDelegate(const TaskType type_, const TaskMask mask_,
            _In_ const ScheduleStageBarrier srcStage_,
            _In_ const ScheduleStageBarrier dstStage_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 16.11.2020
         */
        ~TaskDelegate() = default;

    private:
        /**
         * Make TaskDelegate public unassignable to any type
         */

        TaskDelegate(const TaskDelegate&) noexcept = default;

        TaskDelegate(TaskDelegate&&) noexcept = default;

    private:
        ref<TaskDelegate> operator=(cref<TaskDelegate>) = delete;

        ref<TaskDelegate> operator=(mref<TaskDelegate>) noexcept = delete;
    };

    typedef const TaskDelegate* __TaskDelegate;

    class Task :
        public TaskDelegate {
    public:
        using function_type = _STD function<void()>;

        /**
         * Destructor
         *
         * @author Julius
         * @date 13.11.2020
         */
        ~Task() noexcept;

        /**
         * Executes this wrapped operation
         *
         * @author Julius
         * @date 16.11.2020
         */
        void invoke() const;

    private:
        /**
         * The wrapped function
         */
        function_type _fnc;

        friend __TaskDelegate make_task(function_type&&, TaskMask,
            const ScheduleStageBarrier,
            const ScheduleStageBarrier);

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param [in] fnc_ The function.
         * @param 	   mask_ The mask.
         * @param srcStage_ (Optional) The schedule stage barrier, when to execute the task.
         * @param dstStage_ (Optional) The schedule stage barrier, when the task should be completed.
         */
        Task(IN _STD function<void()>&& fnc_, TaskMask mask_,
            _In_ const ScheduleStageBarrier srcStage_,
            _In_ const ScheduleStageBarrier dstStage_) noexcept;
    };

    /**
     * Make Constructor for [Task]
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in,out] fnc_ The function.
     * @param 		   mask_ (Optional) The mask.
     * @param srcStage_ (Optional) The schedule stage barrier, when to execute the task.
     * @param dstStage_ (Optional) The schedule stage barrier, when the task should be completed.
     */
    __TaskDelegate make_task(Task::function_type&& fnc_, TaskMask mask_ = TaskMask::eNormal,
        _In_ const ScheduleStageBarrier srcStage_ = ScheduleStageBarriers::eAll,
        _In_ const ScheduleStageBarrier dstStage_ = ScheduleStageBarriers::eUndefined);

    class RepetitiveTask :
        public TaskDelegate {
    public:
        using function_type = _STD function<bool()>;

        /**
         * Destructor
         *
         * @author Julius
         * @date 14.11.2020
         */
        ~RepetitiveTask() noexcept;

        /**
         * Executes this wrapper operation
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool invoke() const;

    private:
        /**
         * The wrapped function
         */
        function_type _fnc;

        friend __TaskDelegate make_repetitive_task(function_type&&, TaskMask,
            const ScheduleStageBarrier,
            const ScheduleStageBarrier);

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param [in,out] fnc_ The function.
         * @param 		   mask_ The mask.
         * @param srcStage_ (Optional) The schedule stage barrier, when to execute the task.
         * @param dstStage_ (Optional) The schedule stage barrier, when the task should be completed.
         */
        RepetitiveTask(function_type&& fnc_, TaskMask mask_,
            _In_ const ScheduleStageBarrier srcStage_,
            _In_ const ScheduleStageBarrier dstStage_) noexcept;
    };

    /**
     * Make Constructor for [RepetitiveTask]
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in,out] fnc_ The function.
     * @param 		   mask_ (Optional) The mask.
     * @param srcStage_ (Optional) The schedule stage barrier, when to execute the task.
     * @param dstStage_ (Optional) The schedule stage barrier, when the task should be completed.
     */
    __TaskDelegate make_repetitive_task(RepetitiveTask::function_type&& fnc_, TaskMask mask_ = TaskMask::eNormal,
        _In_ const ScheduleStageBarrier srcStage_ = ScheduleStageBarriers::eAll,
        _In_ const ScheduleStageBarrier dstStage_ = ScheduleStageBarriers::eUndefined);

    class BatchTask :
        public TaskDelegate {
    public:
        using function_type = _STD function<void(ember::u32)>;

        /**
         * Destructor
         *
         * @author Julius
         * @date 14.11.2020
         */
        ~BatchTask() noexcept;

        /**
         * Executes this wrapper operation
         *
         * @author Julius
         * @date 16.11.2020
         */
        void invoke() const;

    private:
        /**
         * The wrapped function
         */
        function_type _fnc;

        friend __TaskDelegate make_batch_task(function_type&&, TaskMask,
            const ScheduleStageBarrier,
            const ScheduleStageBarrier);

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param [in,out] fnc_ The function.
         * @param 		   mask_ The mask.
         * @param srcStage_ (Optional) The schedule stage barrier, when to execute the task.
         * @param dstStage_ (Optional) The schedule stage barrier, when the task should be completed.
         */
        BatchTask(function_type&& fnc_, TaskMask mask_,
            _In_ const ScheduleStageBarrier srcStage_,
            _In_ const ScheduleStageBarrier dstStage_) noexcept;
    };

    /**
     * Make Constructor for [BatchTask]
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in,out] fnc_ The function.
     * @param 		   mask_ (Optional) The mask.
     * @param srcStage_ (Optional) The schedule stage barrier, when to execute the task.
     * @param dstStage_ (Optional) The schedule stage barrier, when the task should be completed.
     */
    __TaskDelegate make_batch_task(BatchTask::function_type&& fnc_, TaskMask mask_ = TaskMask::eNormal,
        _In_ const ScheduleStageBarrier srcStage_ = ScheduleStageBarriers::eAll,
        _In_ const ScheduleStageBarrier dstStage_ = ScheduleStageBarriers::eUndefined);
}
