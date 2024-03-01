#include "Win32DragDropSender.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Platform/Platform.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "Win32DragDropObject.hpp"

using namespace hg::engine::input;
using namespace hg;

Win32DragDropSender::Win32DragDropSender() :
    DragDropSender(),
    _grfKeyState(NULL),
    _cur(nullptr),
    _useCount(1uL) {}

Win32DragDropSender::~Win32DragDropSender() noexcept = default;

void Win32DragDropSender::setup() {}

void Win32DragDropSender::destroy() {}

void Win32DragDropSender::dispatchDragDrop() {
    Engine::getEngine()->getPlatform()->platformQueue().try_push(
        [this]() {
            startDragDrop();
        }
    );
}

void Win32DragDropSender::startDragDrop() {

    #define IS_DOWN(Key_) (GetKeyState(Key_) & 0x8000)
    #define IS_DOT(Key_) (GetKeyState(Key_) & 0x8001)
    #define IS_TOGGLE(Key_) (GetKeyState(Key_) & 0x0001)
    #define IS_UP(Key_) (GetKeyState(Key_) == 0x0)

    // We need to capture current grfKeyState to track changes
    decltype(_grfKeyState) baseState {};
    if (IS_DOWN(VK_CONTROL)) {
        baseState |= MK_CONTROL;
    }

    if (IS_DOWN(VK_SHIFT)) {
        baseState |= MK_SHIFT;
    }

    if (GetKeyState(VK_MENU) & 0x8000/* ALT */) {
        baseState |= MK_ALT;
    }

    if (IS_DOWN(VK_LBUTTON)) {
        baseState |= MK_LBUTTON;
    }

    if (IS_DOWN(VK_MBUTTON)) {
        baseState |= MK_MBUTTON;
    }

    if (IS_DOWN(VK_RBUTTON)) {
        baseState |= MK_RBUTTON;
    }

    _grfKeyState = baseState;

    #undef IS_DOWN
    #undef IS_DOT
    #undef IS_TOGGLE
    #undef IS_UP

    /**/

    DWORD resEffect { NULL };
    const auto result = DoDragDrop(static_cast<ptr<Win32DragDropObject>>(_cur), this, DROPEFFECT_COPY, &resEffect);

    if (result == DRAGDROP_S_CANCEL) {
        tidyDragDrop();
        return;
    }

    if (result == S_OK || result == DRAGDROP_S_DROP) {
        // TODO:
        tidyDragDrop();
        return;
    }

    if (result == E_UNEXPECTED || result) {
        // TODO:
        tidyDragDrop();
        return;
    }
}

void Win32DragDropSender::tidyDragDrop() {

    if (not _cur) {
        return;
    }

    if (static_cast<ptr<Win32DragDropObject>>(_cur)->useCount() >= 1uL) {
        IM_CORE_WARN("Cannot cleanup drag-drop process due so missing resource usage releases");
        // TODO:
    }

    delete _cur;
    _cur = nullptr;
}

void Win32DragDropSender::sendDragFiles(Vector<string> paths_) {

    if (_cur != nullptr) {
        IM_CORE_ERROR("Cannot send another file(set) via drag/clipboard sender while still in progress.");
        return;
    }

    _cur = new Win32DragDropObject();
    _cur->storeFiles(paths_);
    static_cast<ptr<Win32DragDropObject>>(_cur)->AddRef();

    /**/
    dispatchDragDrop();
}

void Win32DragDropSender::sendDragText(cref<string> text_) {

    if (_cur != nullptr) {
        IM_CORE_ERROR("Cannot send another text via drag/clipboard sender while still in progress.");
        return;
    }

    _cur = new Win32DragDropObject();
    _cur->storeText(text_);
    static_cast<ptr<Win32DragDropObject>>(_cur)->AddRef();

    /**/
    dispatchDragDrop();
}

HRESULT Win32DragDropSender::QueryInterface(const IID& riid, void** ppvObject) {

    if (riid == IID_IUnknown) {
        *ppvObject = static_cast<ptr<IUnknown>>(this);
        return S_OK;
    }

    if (riid == IID_IDropSource) {
        *ppvObject = static_cast<ptr<IDropSource>>(this);
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG Win32DragDropSender::AddRef() {
    return ++_useCount;
}

ULONG Win32DragDropSender::Release() {
    return --_useCount;
}

HRESULT Win32DragDropSender::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) {

    if (fEscapePressed) {
        return DRAGDROP_S_CANCEL;
    }

    if (_grfKeyState != grfKeyState) {
        _grfKeyState = NULL;
        return DRAGDROP_S_DROP;
    }

    return S_OK;
}

HRESULT Win32DragDropSender::GiveFeedback(DWORD dwEffect) {
    return DRAGDROP_S_USEDEFAULTCURSORS;
}
