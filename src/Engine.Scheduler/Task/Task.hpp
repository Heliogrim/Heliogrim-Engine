#pragma once

#include <functional>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

#include "TaskMask.hpp"
#include "TaskType.hpp"
#include "TaskCtrl.hpp"

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
        using this_type = TaskDelegate;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param  type_ The type.
         * @param  mask_ The mask.
         */
        TaskDelegate(const TaskType type_, const TaskMask mask_) noexcept;

    public:
        TaskDelegate(const TaskDelegate&) = delete;

        TaskDelegate(TaskDelegate&&) noexcept = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 16.11.2020
         */
        ~TaskDelegate() = default;

    private:
        ref<TaskDelegate> operator=(cref<TaskDelegate>) = delete;

        ref<TaskDelegate> operator=(mref<TaskDelegate>) noexcept = delete;

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

    private:
        ptr<TaskCtrl> _ctrl;

    public:
        [[nodiscard]] ptr<TaskCtrl> ctrl() const noexcept;

        [[nodiscard]] ref<ptr<TaskCtrl>> ctrl() noexcept;
    };

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

        friend const non_owning_rptr<const TaskDelegate> make_task(function_type&&, TaskMask);

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param fnc_ The function.
         * @param mask_ The mask.
         */
        Task(_In_ function_type&& fnc_, TaskMask mask_) noexcept;
    };

    /**
     * Make Constructor for [Task]
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param fnc_ The function.
     * @param mask_ (Optional) The mask.
     */
    const non_owning_rptr<const TaskDelegate> make_task(
        _In_ Task::function_type&& fnc_,
        TaskMask mask_ = TaskMask::eNormal
    );

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

        friend const non_owning_rptr<const TaskDelegate> make_repetitive_task(function_type&&, TaskMask);

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param fnc_ The function.
         * @param mask_ The mask.
         */
        RepetitiveTask(_In_ function_type&& fnc_, TaskMask mask_) noexcept;
    };

    /**
     * Make Constructor for [RepetitiveTask]
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param fnc_ The function.
     * @param mask_ (Optional) The mask.
     */
    const non_owning_rptr<const TaskDelegate> make_repetitive_task(
        _In_ RepetitiveTask::function_type&& fnc_,
        TaskMask mask_ = TaskMask::eNormal
    );

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

        friend const non_owning_rptr<const TaskDelegate> make_batch_task(function_type&&, TaskMask);

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param fnc_ The function.
         * @param mask_ The mask.
         */
        BatchTask(_In_ function_type&& fnc_, TaskMask mask_) noexcept;
    };

    /**
     * Make Constructor for [BatchTask]
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param fnc_ The function.
     * @param mask_ (Optional) The mask.
     */
    const non_owning_rptr<const TaskDelegate> make_batch_task(
        _In_ BatchTask::function_type&& fnc_,
        TaskMask mask_ = TaskMask::eNormal
    );
}
