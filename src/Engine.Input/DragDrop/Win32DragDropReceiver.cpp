#include "Win32DragDropReceiver.hpp"

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Make.hpp>

#include <sstream>
#include <locale>
#include <format>
#include <codecvt>
#include <ShlObj_core.h>

#include "../DragDropEvent.hpp"

using namespace hg::engine::input;
using namespace hg;

Win32DragDropReceiver::Win32DragDropReceiver(const HWND hwnd_) :
    DragDropReceiver(),
    _hwnd(hwnd_),
    _cursorPos(-1i32, -1i32),
    _data(),
    _valid(false),
    _useCount(1ui32) {}

Win32DragDropReceiver::~Win32DragDropReceiver() noexcept = default;

void Win32DragDropReceiver::setup() {
    const auto result = ::RegisterDragDrop(_hwnd, this);
    assert(result == S_OK);
}

void Win32DragDropReceiver::destroy() {
    ::RevokeDragDrop(_hwnd);
}

HRESULT Win32DragDropReceiver::QueryInterface(const IID& riid, void** ppvObject) {

    if (riid == IID_IUnknown) {
        *ppvObject = static_cast<ptr<IUnknown>>(this);
        return S_OK;
    }

    if (riid == IID_IDropTarget) {
        *ppvObject = static_cast<ptr<IDropTarget>>(this);
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG Win32DragDropReceiver::AddRef() {
    return ++_useCount;
}

ULONG Win32DragDropReceiver::Release() {
    return --_useCount;
}

bool Win32DragDropReceiver::supportedDataObject(ptr<IDataObject> data_) const {

    FORMATETC textFormat { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    FORMATETC hdropFormat { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

    /**/
    if (_data->QueryGetData(&textFormat) == S_OK) {
        return true;
    }

    if (_data->QueryGetData(&hdropFormat) == S_OK) {
        return true;
    }

    return false;
}

HRESULT Win32DragDropReceiver::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {

    _cursorPos.x = pt.x;
    _cursorPos.y = pt.y;

    _data = pDataObj;
    _data->AddRef();

    /**/

    if ((_valid = supportedDataObject(_data)) == true) {
        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }

    *pdwEffect = DROPEFFECT_NONE;
    return S_OK;
}

HRESULT Win32DragDropReceiver::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {

    _cursorPos.x = pt.x;
    _cursorPos.y = pt.y;

    /**/

    if (_valid) {
        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }

    *pdwEffect = DROPEFFECT_NONE;
    return S_OK;
}

HRESULT Win32DragDropReceiver::DragLeave() {

    _cursorPos.x = -1;
    _cursorPos.y = -1;

    _data->Release();
    _data = nullptr;

    _valid = false;
    return S_OK;
}

static Vector<string> handleFileDrop(ptr<IDataObject> data_, cref<FORMATETC> format_, ref<STGMEDIUM> medium_) {

    Vector<string> files {};

    switch (medium_.tymed) {
        case TYMED_HGLOBAL: {

            LPVOID dataPtr { GlobalLock(medium_.hGlobal) };
            LPDROPFILES dropFiles { static_cast<LPDROPFILES>(dataPtr) };

            _STD wstringstream ss {};
            const wchar_t* base {
                reinterpret_cast<ptr<wchar_t>>(reinterpret_cast<ptr<char>>(dropFiles) + dropFiles->pFiles)
            };

            u8 terminating { 0ui8 };
            u64 idx { 0ui64 };

            while (terminating < 2ui8) {

                if (base[idx] == '\0') {

                    if (not ss.view().empty()) {

                        const auto dstSize = WideCharToMultiByte(
                            CP_UTF8,
                            NULL,
                            ss.view().data(),
                            ss.view().size(),
                            NULL,
                            0,
                            NULL,
                            NULL
                        );

                        _STD string fileName(dstSize, (NULL));
                        WideCharToMultiByte(
                            CP_UTF8,
                            NULL,
                            ss.view().data(),
                            ss.view().size(),
                            fileName.data(),
                            dstSize,
                            NULL,
                            NULL
                        );

                        files.push_back(fileName);
                        ss.str({});
                    }

                    ++terminating;

                } else {
                    ss << base[idx];
                    terminating = 0;
                }

                ++idx;
            }

            GlobalUnlock(medium_.hGlobal);
            ReleaseStgMedium(&medium_);
            break;
        }
    }

    /**/

    return files;
}

static string handleTextDrop(ptr<IDataObject> data_, cref<FORMATETC> format_, ref<STGMEDIUM> medium_) {

    string text {};

    switch (medium_.tymed) {
        case TYMED_HGLOBAL: {

            LPVOID dataPtr { GlobalLock(medium_.hGlobal) };
            ptr<wchar_t> input { static_cast<ptr<wchar_t>>(dataPtr) };

            const auto dstSize = WideCharToMultiByte(
                CP_UTF8,
                NULL,
                input,
                -1,
                NULL,
                0,
                NULL,
                NULL
            );

            text.clear();
            text.append(dstSize, (NULL));

            WideCharToMultiByte(
                CP_UTF8,
                NULL,
                input,
                -1,
                text.data(),
                dstSize,
                NULL,
                NULL
            );

            text.pop_back();

            GlobalUnlock(medium_.hGlobal);
            ReleaseStgMedium(&medium_);
            break;
        }
    }

    /**/

    return text;
}

HRESULT Win32DragDropReceiver::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {

    _cursorPos.x = pt.x;
    _cursorPos.y = pt.y;

    pDataObj->AddRef();

    _data->Release();
    _data = pDataObj;

    /**/

    STGMEDIUM medium {};

    FORMATETC hdropFormat {
        CF_HDROP,
        nullptr,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    FORMATETC textFormat {
        CF_UNICODETEXT,
        nullptr,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    /**/

    RECT rect {};
    GetWindowRect(_hwnd, &rect);

    uptr<input::event::DragDropEvent> event {
        make_uptr<input::event::DragDropEvent>(
            _cursorPos - math::ivec2 { rect.left, rect.top },
            event::DragDropEventType::eNone,
            event::DragDropEventPayload { nullptr }
        )
    };

    /**/

    if (_data->QueryGetData(&hdropFormat) == S_OK) {

        const auto result { _data->GetData(&hdropFormat, &medium) };
        if (result == S_OK) {

            auto files = handleFileDrop(_data, hdropFormat, medium);

            if (not files.empty()) {
                event->_type = event::DragDropEventType::eFileType;
                event->_data.files = new event::DragDropEventFilePayload { _STD move(files) };
            }
        }

    } else if (_data->QueryGetData(&textFormat) == S_OK) {

        const auto result { _data->GetData(&textFormat, &medium) };
        if (result == S_OK) {

            auto text = handleTextDrop(_data, textFormat, medium);

            if (not text.empty()) {
                event->_type = event::DragDropEventType::eTextType;
                event->_data.text = new event::DragDropEventTextPayload { _STD move(text) };
            }
        }

    }

    /**/

    if (event->_type == event::DragDropEventType::eNone) {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    /**/

    if (_onDrop && _onDrop(_STD move(event))) {
        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }

    *pdwEffect = DROPEFFECT_NONE;
    return S_OK;
}
