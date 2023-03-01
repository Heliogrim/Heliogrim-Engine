#pragma once
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Common/Functional/Function.hpp>

#include "HeliogrimStruct.hpp"
#include "Engine.Common/Exception/NotImplementedException.hpp"
#include "__fwd.hpp"

namespace hg {
    template <class ClassType_>
    concept IsHeliogrimObject = _STD is_base_of_v<HeliogrimObject, ClassType_>;

    template <class FinalClassType_>
    concept IsFinalHeliogrimObject = IsHeliogrimObject<FinalClassType_> && _STD is_final_v<FinalClassType_>;

    class __declspec(novtable) HeliogrimObject {
        using this_type = HeliogrimObject;

    private:
        // TODO: const ptr<const HeliogrimClass> _class = ...;
        ptr<HeliogrimClass> _class = nullptr;

    public:
        [[nodiscard]] FORCE_INLINE const ptr<const HeliogrimClass> getClass() const noexcept {
            return _class;
        }

    public:
        template <IsHeliogrimObject ClassType_, typename... Args_>
        [[nodiscard]] static ptr<ClassType_> create(Args_&&... args_);

        template <IsHeliogrimObject ClassType_, typename... Args_>
        [[nodiscard]] static ptr<ClassType_> createInPlace(const ptr<void> pos_, Args_&&... args_);

        template <IsHeliogrimObject ClassType_>
        static void destroy(mref<ptr<ClassType_>> obj_) {
            delete obj_;
            obj_ = nullptr;
        }

        template <IsHeliogrimObject ClassType_>
        static void destroyInPlace(const ptr<ClassType_> obj_) {
            obj_->~ClassType_();
        }
    };

    class HeliogrimClass final :
        public HeliogrimStruct {
    public:
        using this_type = HeliogrimClass;
        using underlying_type = HeliogrimStruct;

        friend class HeliogrimObject;

    protected:
        HeliogrimClass();

    public:
        ~HeliogrimClass() noexcept = default;

    protected:
        // Warning: Change static singleton behaviour of class definitions
        template <typename Type_>
        [[nodiscard]] static sptr<HeliogrimClass> __makeFromType() {

            auto ec { sptr<HeliogrimClass> { new HeliogrimClass() } };
            ec->typeId() = HeliogrimClass::stid<Type_>();
            ec->capture<Type_>();

            return ec;
        }

        template <typename Type_>
        [[nodiscard]] static ptr<HeliogrimClass> __getOrCreate() {
            static sptr<HeliogrimClass> ec { __makeFromType<Type_>() };
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
        [[nodiscard]] static ptr<HeliogrimClass> of() noexcept {
            if constexpr (IsHeliogrimObject<ClassType_>) {
                return HeliogrimClass::__getOrCreate<ClassType_>();
            }
            return nullptr;
        }

        template <class ClassType_>
        [[nodiscard]] static ptr<HeliogrimClass> of(cref<ClassType_> obj_) noexcept {
            if constexpr (IsHeliogrimObject<ClassType_>) {
                return obj_.getClass();
            }
            return of<ClassType_>();
        }

        template <class ClassType_>
        [[nodiscard]] static ptr<HeliogrimClass> of(const ptr<const ClassType_> obj_) noexcept {
            if constexpr (IsHeliogrimObject<ClassType_>) {
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
        template <IsHeliogrimObject TargetType_>
        [[nodiscard]] bool isType() const noexcept {
            if constexpr (IsFinalHeliogrimObject<TargetType_>) {
                return this == HeliogrimClass::__getOrCreate<TargetType_>();
            } else {
                throw NotImplementedException();
            }
        }

        template <IsHeliogrimObject TargetType_>
        [[nodiscard]] bool isExactType() const noexcept {
            return this == HeliogrimClass::__getOrCreate<TargetType_>();
        }

    private:
        nular_fnc<ptr<HeliogrimObject>> _rnctor;
        unary_fnc<void, mref<ptr<HeliogrimObject>>> _rndtor;

        unary_fnc<ptr<HeliogrimObject>, ptr<void>> _rctor;
        unary_fnc<void, const ptr<HeliogrimObject>> _rdtor;

    private:
        template <typename Type_>
        void capture();

    public:
        [[nodiscard]] ptr<HeliogrimObject> instantiate() const;

        void destroy(mref<ptr<HeliogrimObject>> obj_) const;

        [[nodiscard]] ptr<HeliogrimObject> construct(ptr<void> dst_) const;

        void destruct(ptr<HeliogrimObject> obj_) const;
    };

    template <IsHeliogrimObject ClassType_, typename... Args_>
    ptr<ClassType_> HeliogrimObject::create(Args_&&... args_) {
        auto* obj { new ClassType_(_STD forward<Args_>(args_)...) };
        obj->_class = HeliogrimClass::of<ClassType_>();
        return obj;
    }

    template <IsHeliogrimObject ClassType_, typename... Args_>
    ptr<ClassType_> HeliogrimObject::createInPlace(const ptr<void> pos_, Args_&&... args_) {
        auto* obj { new(pos_) ClassType_(_STD forward<Args_>(args_)...) };
        obj->_class = HeliogrimClass::of<ClassType_>();
        return obj;
    }

    template <typename Type_>
    void HeliogrimClass::capture() {

        if constexpr (_STD is_default_constructible_v<Type_>) {
            _rctor = reinterpret_cast<unary_fnc<ptr<HeliogrimObject>, ptr<void>>>(HeliogrimObject::createInPlace<Type_>);
            _rnctor = reinterpret_cast<nular_fnc<ptr<HeliogrimObject>>>(HeliogrimObject::create<Type_>);
        } else {
            _rctor = nullptr;
            _rnctor = nullptr;
        }

        if constexpr (_STD is_destructible_v<Type_>) {
            _rdtor = reinterpret_cast<unary_fnc<void, const ptr<HeliogrimObject>>>(HeliogrimObject::destroyInPlace<Type_>);
            _rndtor = reinterpret_cast<unary_fnc<void, mref<ptr<HeliogrimObject>>>>(HeliogrimObject::destroy<Type_>);
        } else {
            _rdtor = nullptr;
            _rndtor = nullptr;
        }
    }
}
