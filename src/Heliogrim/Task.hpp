#pragma once

#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Scheduler/Task/TaskMask.hpp>
#include <Engine.Scheduler/Task/TaskType.hpp>

namespace hg {
    using TaskMask = engine::scheduler::task::TaskMask;
    using TaskType = engine::scheduler::task::TaskType;

    class TaskBase {
    public:
        using this_type = TaskBase;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @param type_ The type mask entry for this task
         */
        TaskBase(cref<TaskType> type_) noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 20.05.2022
         *
         * @param other_ The instance to move.
         */
        TaskBase(mref<TaskBase> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.11.2021
         */
        ~TaskBase() noexcept;

    protected:
        /**
         * The tasks type identifier
         */
        TaskType _type;

    public:
        /**
         * Get the tasks type
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns The TaskType of this task.
         */
        [[nodiscard]] TaskType type() const noexcept;

    protected:
        /**
         * The masking for this task
         */
        TaskMask _mask;

    public:
        /**
         * Get the tasks mask
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns The TaskMask of this task.
         */
        [[nodiscard]] TaskMask mask() const noexcept;

        /**
         * Get the tasks mask
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns A reference to the TaskMask of this task.
         */
        [[nodiscard]] ref<TaskMask> mask() noexcept;
    };

    struct Task final : public TaskBase {
    public:
        using this_type = Task;
        using underlying_type = TaskBase;

        using function_type = _STD function<void()>;

    public:
        /**
         * Default Constructor
         *
         * @author Julius
         * @date 20.11.2021
         */
        Task() noexcept;

        /**
         * Constructor
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @param fnc_ The function to handle as task
         */
        Task(mref<function_type> fnc_) noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @param other_ The task to copy from
         */
        Task(cref<Task> other_) noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @param other_ The task to move from
         */
        Task(mref<Task> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.11.2021
         */
        ~Task() noexcept;

    private:
        /**
         * The function handled as task
         */
        function_type _fnc;

    public:
        /**
         * Get the handled function
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns A const reference to the handled function
         */
        [[nodiscard]] cref<function_type> fnc() const noexcept;

        /**
         * Get the handled function
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns A reference to the handled function
         */
        [[nodiscard]] ref<function_type> fnc() noexcept;
    };

    struct RepetitiveTask final : public TaskBase {
    public:
        using this_type = RepetitiveTask;
        using underlying_type = TaskBase;

        using function_type = _STD function<bool()>;

    public:
        /**
         * Default Constructor
         *
         * @author Julius
         * @date 20.11.2021
         */
        RepetitiveTask() noexcept;

        /**
         * Constructor
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @param fnc_ The function to handle as task
         */
        RepetitiveTask(mref<function_type> fnc_) noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @param other_ The repetitive task to copy from
         */
        RepetitiveTask(cref<RepetitiveTask> other_) noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @param other_ The repetitive task to move from
         */
        RepetitiveTask(mref<RepetitiveTask> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.11.2021
         */
        ~RepetitiveTask() noexcept;

    private:
        /**
         * The function handled as task
         */
        function_type _fnc;

    public:
        /**
         * Get the handled function
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns A const reference to the handled function
         */
        [[nodiscard]] cref<function_type> fnc() const noexcept;

        /**
         * Get the handled function
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns A reference to the handled function
         */
        [[nodiscard]] ref<function_type> fnc() noexcept;
    };

    struct TickTask final : public TaskBase {
    public:
        using this_type = TickTask;
        using underlying_type = TaskBase;

        using function_type = _STD function<void()>;

    public:
        /**
         * Default Constructor
         *
         * @author Julius
         * @date 20.11.2021
         */
        TickTask() noexcept;

        /**
         * Constructor
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @param fnc_ The function to handle as task
         */
        TickTask(mref<function_type> fnc_) noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @param other_ The tick task to copy from
         */
        TickTask(cref<TickTask> other_) noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @param other_ The tick task to move from
         */
        TickTask(mref<TickTask> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.11.2021
         */
        ~TickTask() noexcept;

    private:
        /**
         * The function handled as task
         */
        function_type _fnc;

    public:
        /**
         * Get the handled function
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns A const reference to the handled function
         */
        [[nodiscard]] cref<function_type> fnc() const noexcept;

        /**
         * Get the handled function
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns A reference to the handled function
         */
        [[nodiscard]] ref<function_type> fnc() noexcept;
    };
}
