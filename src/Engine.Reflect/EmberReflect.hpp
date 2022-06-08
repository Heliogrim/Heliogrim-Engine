#pragma once
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

#include "EmberStruct.hpp"
#include "Engine.Common/Exception/NotImplementedException.hpp"

namespace ember {
    /**
     * Forward Declaration
     */
    class EmberClass;
    class EmberObject;
}

namespace ember {

    template <class ClassType_>
    concept IsEmberObject = _STD is_base_of_v<EmberObject, ClassType_>;

    template <class FinalClassType_>
    concept IsFinalEmberObject = IsEmberObject<FinalClassType_> && _STD is_final_v<FinalClassType_>;

    class __declspec(novtable) EmberObject {
        using this_type = EmberObject;

    private:
        // TODO: const ptr<const EmberClass> _class = ...;
        ptr<EmberClass> _class = nullptr;

    public:
        [[nodiscard]] FORCEINLINE const ptr<const EmberClass> getClass() const noexcept {
            return _class;
        }

    public:
        template <IsEmberObject ClassType_, typename... Args_>
        [[nodiscard]] static ptr<ClassType_> create(Args_&&... args_);

        template <IsEmberObject ClassType_>
        static void destroy(mref<ptr<ClassType_>> obj_) {
            delete obj_;
            obj_ = nullptr;
        }
    };

    class EmberClass final :
        public EmberStruct {
    public:
        using this_type = EmberClass;
        using underlying_type = EmberStruct;

        friend class EmberObject;

    protected:
        EmberClass();

    public:
        ~EmberClass() noexcept = default;

    protected:
        // Warning: Change static singleton behaviour of class definitions
        template <typename Type_>
        [[nodiscard]] static sptr<EmberClass> __makeFromType() {
            auto ec { sptr<EmberClass> { new EmberClass() } };
            ec->typeId() = EmberClass::stid<Type_>();
            return ec;
        }

        template <typename Type_>
        [[nodiscard]] static ptr<EmberClass> __getOrCreate() {
            static sptr<EmberClass> ec { __makeFromType<Type_>() };
            return ec.get();
        }

    private:
        type_id _typeId;

    public:
        [[nodiscard]] cref<type_id> typeId() const noexcept;

    protected:
        [[nodiscard]] ref<type_id> typeId() noexcept;

    public:
        template <class ClassType_>
        [[nodiscard]] static ptr<EmberClass> of() noexcept {
            if constexpr (IsEmberObject<ClassType_>) {
                return EmberClass::__getOrCreate<ClassType_>();
            }
            return nullptr;
        }

        template <class ClassType_>
        [[nodiscard]] static ptr<EmberClass> of(cref<ClassType_> obj_) noexcept {
            if constexpr (IsEmberObject<ClassType_>) {
                return obj_.getClass();
            }
            return of<ClassType_>();
        }

        template <class ClassType_>
        [[nodiscard]] static ptr<EmberClass> of(const ptr<const ClassType_> obj_) noexcept {
            if constexpr (IsEmberObject<ClassType_>) {
                return obj_->getClass();
            }
            return of<ClassType_>();
        }

    public:
        template <class ClassType_>
        [[nodiscard]] static constexpr type_id stid() noexcept {
            if constexpr (HasStaticType<ClassType_, type_id>) {
                return ClassType_::typeId;
            }
            #ifdef _DEBUG
            static_assert(HasStaticType<ClassType_, type_id>, "Failed to determine static type_id while requested.");
            #endif
            return type_id { 0ui64 };
        }

        template <class ClassType_>
        [[nodiscard]] static type_id dtid(cref<ClassType_> obj_) noexcept {
            if constexpr (HasDynamicType<ClassType_, type_id>) {
                return obj_.getTypeId();
            }
            return type_id { 0ui64 };
        }

        template <class ClassType_>
        [[nodiscard]] static type_id dtid(const ptr<const ClassType_> obj_) noexcept {
            if constexpr (HasDynamicType<ClassType_, type_id>) {
                return obj_->getTypeId();
            }
            return type_id { 0ui64 };
        }

        template <class ClassType_>
        [[nodiscard]] static type_id tid(cref<ClassType_> obj_) noexcept {
            if constexpr (HasStaticType<ClassType_, type_id>) {
                return ClassType_::typeId;
            } else if constexpr (HasDynamicType<ClassType_, type_id>) {
                return obj_.getTypeId();
            }
            #ifdef _DEBUG
            static_assert(HasStaticType<ClassType_, type_id> || HasDynamicType<ClassType_, type_id>,
                "Failed to determine <static|dynamic> type_id while requested.");
            #endif
            return type_id { 0ui64 };
        }

        template <class ClassType_>
        [[nodiscard]] static type_id tid(const ptr<const ClassType_> obj_) noexcept {
            if constexpr (HasStaticType<ClassType_, type_id>) {
                return ClassType_::typeId;
            } else if constexpr (HasDynamicType<ClassType_, type_id>) {
                return obj_->getTypeId();
            }
            #ifdef _DEBUG
            static_assert(HasStaticType<ClassType_, type_id> || HasDynamicType<ClassType_, type_id>,
                "Failed to determine <static|dynamic> type_id while requested.");
            #endif
            return type_id { 0ui64 };
        }

    public:
        template <IsEmberObject TargetType_>
        [[nodiscard]] bool isType() const noexcept {
            if constexpr (IsFinalEmberObject<TargetType_>) {
                return this == EmberClass::__getOrCreate<TargetType_>();
            } else {
                throw NotImplementedException();
            }
        }

        template <IsEmberObject TargetType_>
        [[nodiscard]] bool isExactType() const noexcept {
            return this == EmberClass::__getOrCreate<TargetType_>();
        }
    };

    template <IsEmberObject ClassType_, typename... Args_>
    ptr<ClassType_> EmberObject::create(Args_&&... args_) {
        auto* obj { new ClassType_(_STD forward<Args_>(args_)...) };
        obj->_class = EmberClass::of<ClassType_>();
        return obj;
    }

}
