#pragma once

#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Scheduler/Task/TaskMask.hpp>
#include <Engine.Scheduler/Task/TaskType.hpp>
#include <Engine.Scheduler/Stage/ScheduleStage.hpp>

namespace ember {

    using TaskMask = engine::scheduler::task::TaskMask;
    using TaskType = engine::scheduler::task::TaskType;

    using TaskStage = engine::scheduler::ScheduleStageBarrier;

    namespace TaskStages {
        /**
         *
         */
        constexpr auto eUndefined = engine::scheduler::ScheduleStageBarriers::eUndefined;
        /**
         *
         */
        constexpr auto eTopStrong = engine::scheduler::ScheduleStageBarriers::eTopStrong;
        constexpr auto eTopWeak = engine::scheduler::ScheduleStageBarriers::eTopWeak;
        constexpr auto eNetworkFetchStrong = engine::scheduler::ScheduleStageBarriers::eNetworkFetchStrong;
        constexpr auto eNetworkFetchWeak = engine::scheduler::ScheduleStageBarriers::eNetworkFetchWeak;
        constexpr auto eUserUpdateStrong = engine::scheduler::ScheduleStageBarriers::eUserUpdateStrong;
        constexpr auto eUserUpdateWeak = engine::scheduler::ScheduleStageBarriers::eUserUpdateWeak;
        constexpr auto ePublishStrong = engine::scheduler::ScheduleStageBarriers::ePublishStrong;
        constexpr auto ePublishWeak = engine::scheduler::ScheduleStageBarriers::ePublishWeak;
        constexpr auto ePhysicsSimulateStrong = engine::scheduler::ScheduleStageBarriers::ePhysicsSimulateStrong;
        constexpr auto ePhysicsSimulateWeak = engine::scheduler::ScheduleStageBarriers::ePhysicsSimulateWeak;
        constexpr auto eNetworkPushStrong = engine::scheduler::ScheduleStageBarriers::eNetworkPushStrong;
        constexpr auto eNetworkPushWeak = engine::scheduler::ScheduleStageBarriers::eNetworkPushWeak;
        constexpr auto eGraphicNodeCollectStrong = engine::scheduler::ScheduleStageBarriers::eGraphicNodeCollectStrong;
        constexpr auto eGraphicNodeCollectWeak = engine::scheduler::ScheduleStageBarriers::eGraphicNodeCollectWeak;
        constexpr auto eBottomStrong = engine::scheduler::ScheduleStageBarriers::eBottomStrong;
        constexpr auto eBottomWeak = engine::scheduler::ScheduleStageBarriers::eBottomWeak;
        /**
         *
         */
        constexpr auto eAll = engine::scheduler::ScheduleStageBarriers::eAll;
    };

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

    protected:
        TaskStage _srcStage;
        TaskStage _dstStage;

    public:
        /**
         * Get the tasks source stage
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns The TaskStage used as source
         */
        [[nodiscard]] TaskStage srcStage() const noexcept;

        /**
         * Get the tasks source stage
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns A reference to the TaskStage used as source
         */
        [[nodiscard]] ref<TaskStage> srcStage() noexcept;

        /**
         * Get the tasks destination stage
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns The TaskStage used as destination
         */
        [[nodiscard]] TaskStage dstStage() const noexcept;

        /**
         * Get the tasks destination stage
         *
         * @author Julius
         * @date 20.11.2021
         *
         * @returns A reference to the TaskStage used as destination
         */
        [[nodiscard]] ref<TaskStage> dstStage() noexcept;
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
