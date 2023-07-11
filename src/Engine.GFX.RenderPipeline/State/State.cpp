#include "State.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx;
using namespace hg;

State::State(mref<uptr<cache::LocalResourceCache>> cache_, mref<memory::LocalPooledAllocator> allocator_) noexcept :
    _path(),
    _cacheCtrl({ _STD move(cache_) }),
    _gfxAllocator(_STD move(allocator_)),
    _regSubState(),
    _arena(),
    _memList() {}

cref<Vector<nmpt<const Stage>>> State::getPath() const noexcept {
    return _path;
}

ref<cache::LocalCacheCtrl> State::cache() noexcept {
    return _cacheCtrl;
}

ref<memory::LocalPooledAllocator> State::gfxAllocator() noexcept {
    return _gfxAllocator;
}

void State::regSubState(type_id type_, size_t byteSize_, size_t key_) {

    #ifdef _DEBUG
    assert(_arena.mem == nullptr && "Not allowed to change registered substate while already allocated.");
    #endif

    SubStateInfo info { key_, type_, byteSize_, 0 };
    const auto it = _STD ranges::find_if(
        _regSubState,
        [&info](cref<SubStateInfo> entry_) {
            return entry_.key == info.key && entry_.type == info.type;
        }
    );

    if (it != _regSubState.end()) {
        return;
    }

    /**/

    _regSubState.push_back(_STD move(info));

}

void* State::allocSubState(type_id type_, size_t key_) {

    const auto it = _STD ranges::find_if(
        _regSubState,
        [type_, key_](cref<SubStateInfo> info_) {
            return info_.key == key_ && info_.type == type_;
        }
    );

    assert(it != _regSubState.end());

    return static_cast<_::byte*>(_arena.mem) + it->offset;
}

void* State::allocSubState(size_t byteSize_) {
    auto* mem = malloc(byteSize_);
    _memList.push_back(mem);
    return mem;
}

void State::freeSubState(mref<nmpt<void>> ptr_) {

    const ptrdiff_t diff = reinterpret_cast<ptrdiff_t>(ptr_.get()) - reinterpret_cast<ptrdiff_t>(_arena.mem);
    if (diff >= 0 && diff < _arena.size) {
        // TODO: free arena allocation
        ptr_ = nullptr;
        return;
    }

    /**/

    const auto it = _STD ranges::find(_memList, ptr_.get());
    if (it == _memList.end()) {
        assert(false && "Tried to free memory through state allocation, which is not part of the state.");
        return;
    }

    _memList.erase(it);

    free(ptr_.get());
    ptr_ = nullptr;
}
