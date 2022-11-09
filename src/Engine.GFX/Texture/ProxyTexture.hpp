#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include "__fwd.hpp"

namespace ember::engine::gfx {

    namespace {

        enum class ProxyTextureType : u8 {
            eNone = 0x0,
            //
            eTexture = 0x1,
            eTextureView = 0x2,
            //
            eVirtualTexture = 0x3,
            eVirtualTextureView = 0x4
        };

    }

    template <class Type_>
    concept IsProxyTextureType = _STD same_as<Texture, Type_> || _STD same_as<TextureView, Type_> || _STD same_as<
        VirtualTexture, Type_> || _STD same_as<VirtualTextureView, Type_>;

    template <class Type_>
    concept HasResetMethod = requires(Type_& obj_) {
        obj_.reset();
    };

    template <class Type_>
    concept HasGetMethod = requires(Type_& obj_) {
        obj_.get();
    };

    /**
     * Due to the usage of forward declarations even within the templated class itself, we might need to include the external declaration of
     *  underlying possible texture type, cause smart pointers might trigger pointer management (e.g. `virtual deleter´)
     */
    template <template <typename> typename PtrType_>
    class ProxyTexture {
    public:
        using this_type = ProxyTexture<PtrType_>;

        template <typename Type_>
        using ptr_type = PtrType_<Type_>;
        using subject_ptr_type = ptr_type<void>;

    public:
        constexpr ProxyTexture() noexcept :
            _subject(nullptr),
            _subjectType(ProxyTextureType::eNone) {}

        ProxyTexture(mref<ptr_type<Texture>> texture_) :
            _subject(_STD move(texture_)),
            _subjectType(ProxyTextureType::eTexture) {}

        ProxyTexture(mref<ptr_type<TextureView>> textureView_) :
            _subject(_STD move(textureView_)),
            _subjectType(ProxyTextureType::eTextureView) {}

        ProxyTexture(mref<ptr_type<VirtualTexture>> virtualTexture_) :
            _subject(_STD move(virtualTexture_)),
            _subjectType(ProxyTextureType::eVirtualTexture) {}

        ProxyTexture(mref<ptr_type<VirtualTextureView>> virtualTextureView_) :
            _subject(_STD move(virtualTextureView_)),
            _subjectType(ProxyTextureType::eVirtualTextureView) {}

        // TODO: Check whether we can copy underlying pointer type like `ptr` or `sptr` but prevent `uptr`
        ProxyTexture(cref<this_type> other_) :
            _subject(other_._subject),
            _subjectType(other_._subjectType) {}

        ProxyTexture(mref<this_type> other_) :
            _subject(_STD exchange(other_._subject, nullptr)),
            _subjectType(_STD exchange(other_._subjectType, ProxyTextureType::eNone)) {}

        ~ProxyTexture() {
            _tidy();
        }

    private:
        void _tidy() {
            // Warning: We need to check whether this proxy has ownership and is required to free resources
            if constexpr (HasResetMethod<subject_ptr_type>) {
                // TODO: What should we do with probably a smart pointer
                switch (_subjectType) {
                    case ProxyTextureType::eTexture:
                    case ProxyTextureType::eTextureView:
                    case ProxyTextureType::eVirtualTexture:
                    case ProxyTextureType::eVirtualTextureView: __noop;
                }

            } else {
                // TODO: What should we do with a plain pointer
                switch (_subjectType) {
                    case ProxyTextureType::eTexture:
                    case ProxyTextureType::eTextureView:
                    case ProxyTextureType::eVirtualTexture:
                    case ProxyTextureType::eVirtualTextureView: __noop;
                }
            }

            _subject = nullptr;
            _subjectType = ProxyTextureType::eNone;
        }

    private:
        subject_ptr_type _subject;
        ProxyTextureType _subjectType;

    private:
        FORCE_INLINE const ptr<void> unfancy() const noexcept {
            if constexpr (HasGetMethod<subject_ptr_type>) {
                return _subject.get();
            } else if constexpr (_STD is_pointer_v<subject_ptr_type>) {
                return _subject;
            } else {
                return _STD _Unfancy(_subject);
            }
        }

    public:
        template <IsProxyTextureType Type_>
        [[nodiscard]] const ptr<Type_> as() const noexcept {
            return nullptr;
        }

        template <>
        [[nodiscard]] const ptr<Texture> as() const noexcept {
            if (_subjectType == ProxyTextureType::eTexture) {
                return static_cast<const ptr<Texture>>(unfancy());
            }
            return nullptr;
        }

        template <>
        [[nodiscard]] const ptr<TextureView> as() const noexcept {
            if (_subjectType == ProxyTextureType::eTextureView) {
                return static_cast<const ptr<TextureView>>(unfancy());
            }
            return nullptr;
        }

        template <>
        [[nodiscard]] const ptr<VirtualTexture> as() const noexcept {
            if (_subjectType == ProxyTextureType::eVirtualTexture) {
                return static_cast<const ptr<VirtualTexture>>(unfancy());
            }
            return nullptr;
        }

        template <>
        [[nodiscard]] const ptr<VirtualTextureView> as() const noexcept {
            if (_subjectType == ProxyTextureType::eVirtualTextureView) {
                return static_cast<const ptr<VirtualTextureView>>(unfancy());
            }
            return nullptr;
        }

    public:
        template <IsProxyTextureType Type_>
        [[nodiscard]] bool is() const noexcept {
            return false;
        }

        template <>
        [[nodiscard]] bool is<Texture>() const noexcept {
            return _subjectType == ProxyTextureType::eTexture;
        }

        template <>
        [[nodiscard]] bool is<TextureView>() const noexcept {
            return _subjectType == ProxyTextureType::eTextureView;
        }

        template <>
        [[nodiscard]] bool is<VirtualTexture>() const noexcept {
            return _subjectType == ProxyTextureType::eVirtualTexture;
        }

        template <>
        [[nodiscard]] bool is<VirtualTextureView>() const noexcept {
            return _subjectType == ProxyTextureType::eVirtualTextureView;
        }

    public:
        // TODO: Check whether we can copy underlying pointer type like `ptr` or `sptr` but prevent `uptr`
        ref<this_type> operator=(cref<this_type> other_) {
            if (this != _STD addressof(other_)) {
                _subjectType = other_._subjectType;
                _subject = other_._subject;
            }
            return *this;
        }

        ref<this_type> operator=(mref<this_type> other_) {
            if (this != _STD addressof(other_)) {
                _subjectType = _STD exchange(other_._subjectType, ProxyTextureType::eNone);
                _subject = _STD exchange(other_._subject, nullptr);
            }
            return *this;
        }

    public:
        ref<this_type> operator=(mref<ptr_type<Texture>> texture_) {

            /*
            if (_STD addressof(_subject) == _STD addressof(texture_)) {
                return *this;
            }
             */

            _tidy();

            _subject = _STD move(texture_);
            _subjectType = ProxyTextureType::eTexture;

            return *this;
        }

        ref<this_type> operator=(mref<ptr_type<TextureView>> texture_) {

            /*
            if (_STD addressof(_subject) == _STD addressof(texture_)) {
                return *this;
            }
             */

            _tidy();

            _subject = _STD move(texture_);
            _subjectType = ProxyTextureType::eTextureView;

            return *this;
        }

        ref<this_type> operator=(mref<ptr_type<VirtualTexture>> texture_) {

            /*
            if (_STD addressof(_subject) == _STD addressof(texture_)) {
                return *this;
            }
             */

            _tidy();

            _subject = _STD move(texture_);
            _subjectType = ProxyTextureType::eVirtualTexture;

            return *this;
        }

        ref<this_type> operator=(mref<ptr_type<VirtualTextureView>> texture_) {

            /*
            if (_STD addressof(_subject) == _STD addressof(texture_)) {
                return *this;
            }
             */

            _tidy();

            _subject = _STD move(texture_);
            _subjectType = ProxyTextureType::eVirtualTextureView;

            return *this;
        }
    };

}
