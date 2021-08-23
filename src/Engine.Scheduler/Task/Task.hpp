#pragma once

#include <functional>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/__macro.hpp>

#include "TaskMask.hpp"
#include "TaskType.hpp"

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
        /** The type */
        const TaskType _type;

        /** The mask */
        const TaskMask _mask;

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

        friend __TaskDelegate make_task(function_type&&, TaskMask);

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param [in] fnc_ The function.
         * @param 	   mask_ The mask.
         */
        Task(IN _STD function<void()>&& fnc_, TaskMask mask_) noexcept;
    };

    /**
     * Make Constructor for [Task]
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in,out] fnc_ The function.
     * @param 		   mask_ (Optional) The mask.
     */
    __TaskDelegate make_task(Task::function_type&& fnc_, TaskMask mask_ = TaskMask::eNormal);

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

        friend __TaskDelegate make_repetitive_task(function_type&&, TaskMask);

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param [in,out] fnc_ The function.
         * @param 		   mask_ The mask.
         */
        RepetitiveTask(function_type&& fnc_, TaskMask mask_) noexcept;
    };

    /**
     * Make Constructor for [RepetitiveTask]
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in,out] fnc_ The function.
     * @param 		   mask_ (Optional) The mask.
     */
    __TaskDelegate make_repetitive_task(RepetitiveTask::function_type&& fnc_, TaskMask mask_ = TaskMask::eNormal);

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

        friend __TaskDelegate make_batch_task(function_type&&, TaskMask);

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param [in,out] fnc_ The function.
         * @param 		   mask_ The mask.
         */
        BatchTask(function_type&& fnc_, TaskMask mask_) noexcept;
    };

    /**
     * Make Constructor for [BatchTask]
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in,out] fnc_ The function.
     * @param 		   mask_ (Optional) The mask.
     */
    __TaskDelegate make_batch_task(BatchTask::function_type&& fnc_, TaskMask mask_ = TaskMask::eNormal);
}
