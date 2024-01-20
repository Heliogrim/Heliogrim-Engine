#include "Win32DragDropObject.hpp"

#include <Engine.Logging/Logger.hpp>
#include <sstream>
#include <ShlObj_core.h>

namespace hg::engine::input {
    // @see https://learn.microsoft.com/en-us/windows/win32/shell/clipboard#cf_hdrop
    struct DragDropObjectFilePayload {
        _STD wstring paths;
    };

    // CF_UNICODETEXT (13) | Unicode text | Text | Null Terminated
    struct DragDropObjectTextPayload {
        _STD wstring data;
    };
}

using namespace hg::engine::input;
using namespace hg;

Win32DragDropObject::Win32DragDropObject() :
    DragDropObject(),
    _useCount(1ui32),
    _formats(),
    _mediums() {}

Win32DragDropObject::~Win32DragDropObject() = default;

void Win32DragDropObject::tidy() {

    if (not _formats.empty()) {
        _formats.clear();
    }

    if (not _mediums.empty()) {
        for (auto& entry : _mediums) {
            ::ReleaseStgMedium(&entry);
        }
        _mediums.clear();
    }

}

bool Win32DragDropObject::storeFiles(cref<Vector<string>> paths_) {

    if (_type != DragDropObjectType::eNone || _payload.files != nullptr) {
        IM_CORE_WARN("Tried to override value and value type of drag-drop object instance.");
        return false;
    }

    // @see https://learn.microsoft.com/en-us/windows/win32/shell/clipboard#cf_hdrop
    _type = DragDropObjectType::eFileType;

    _STD wstringstream stream {};
    for (const auto& path : paths_) {

        const auto wsize = MultiByteToWideChar(
            CP_UTF8,
            NULL,
            path.c_str(),
            path.size(),
            nullptr,
            0
        );

        _STD wstring wpath(wsize, NULL);

        MultiByteToWideChar(
            CP_UTF8,
            NULL,
            path.c_str(),
            path.size(),
            wpath.data(),
            wpath.size()
        );

        stream << wpath << wchar_t { '\0' };
    }

    // Insert
    if (stream.view().empty()) {
        stream << wchar_t { '\0' };
    }

    // Store double null terminator
    stream << wchar_t { '\0' };

    /**/

    _payload.files = new DragDropObjectFilePayload { _STD move(stream.str()) };

    FORMATETC format {
        CF_HDROP,
        nullptr,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    STGMEDIUM medium {
        TYMED_HGLOBAL,
        { nullptr },
        nullptr
    };

    /**/

    const auto globalSize { _payload.files->paths.size() * sizeof(wchar_t) + sizeof(DROPFILES) };

    medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE, globalSize);
    LPVOID hdata = GlobalLock(medium.hGlobal);

    auto* const pDropFile = static_cast<LPDROPFILES>(hdata);
    pDropFile->pFiles = sizeof(DROPFILES);
    pDropFile->pt = POINT { ~0, ~0 };
    pDropFile->fNC = FALSE;
    pDropFile->fWide = TRUE;

    _STD memcpy(
        (static_cast<ptr<char>>(hdata) + sizeof(DROPFILES)),
        _payload.files->paths.data(),
        _payload.files->paths.size() * sizeof(wchar_t)
    );

    GlobalUnlock(hdata);

    /**/

    _formats.push_back(format);
    _mediums.push_back(medium);

    return true;
}

bool Win32DragDropObject::storeText(cref<string> text_) {

    if (_type != DragDropObjectType::eNone || _payload.text != nullptr) {
        IM_CORE_WARN("Tried to override value and value type of drag-drop object instance.");
        return false;
    }

    _type = DragDropObjectType::eTextType;

    const auto wsize = MultiByteToWideChar(
        CP_UTF8,
        NULL,
        text_.c_str(),
        text_.size(),
        nullptr,
        0
    );

    _STD wstring wtext(wsize, NULL);

    MultiByteToWideChar(
        CP_UTF8,
        NULL,
        text_.c_str(),
        text_.size(),
        wtext.data(),
        wtext.size()
    );

    wtext.append(1, '\0');

    /**/

    _payload.text = new DragDropObjectTextPayload { _STD move(wtext) };

    FORMATETC format {
        CF_UNICODETEXT,
        nullptr,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    STGMEDIUM medium {
        TYMED_HGLOBAL,
        { nullptr },
        nullptr
    };

    /**/

    medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE, _payload.text->data.size() * sizeof(wchar_t));

    LPVOID hdata = GlobalLock(medium.hGlobal);
    _STD memcpy(hdata, _payload.text->data.data(), _payload.text->data.size() * sizeof(wchar_t));
    GlobalUnlock(hdata);

    /**/

    _formats.push_back(format);
    _mediums.push_back(medium);

    return true;
}

u32 Win32DragDropObject::useCount() const noexcept {
    return _useCount.load();
}

HRESULT Win32DragDropObject::QueryInterface(const IID& riid, void** ppvObject) {

    if (riid == IID_IUnknown) {
        *ppvObject = static_cast<ptr<IUnknown>>(static_cast<ptr<IDataObject>>(this));
        return S_OK;
    }

    if (riid == IID_IDataObject) {
        *ppvObject = static_cast<ptr<IDataObject>>(this);
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG Win32DragDropObject::AddRef() {
    return ++_useCount;
}

ULONG Win32DragDropObject::Release() {
    return --_useCount;
}

s64 Win32DragDropObject::queryFormatIdx(ptr<FORMATETC> format_) const noexcept {

    for (s64 i { 0i64 }; i < _formats.size(); ++i) {
        if (
            _formats[i].cfFormat == format_->cfFormat &&
            _formats[i].dwAspect == format_->dwAspect &&
            (_formats[i].tymed & format_->tymed) == _formats[i].tymed
        ) {
            return i;
        }
    }

    return -1i64;
}

HGLOBAL Win32DragDropObject::makeHGlobalCopy(HGLOBAL src_) {

    const auto length { GlobalSize(src_) };
    const PVOID pSrc { GlobalLock(src_) };

    auto* const dst { GlobalAlloc(GMEM_MOVEABLE, length) };
    const PVOID pDst { GlobalLock(dst) };

    memcpy(pDst, pSrc, length);
    GlobalUnlock(src_);

    GlobalUnlock(dst);
    return dst;
}

HRESULT Win32DragDropObject::GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium) {

    s64 formatIdx { -1i64 };
    if ((formatIdx = queryFormatIdx(pformatetcIn)) == -1i64) {
        return DV_E_FORMATETC;
    }

    pmedium->tymed = _formats[formatIdx].tymed;
    pmedium->pUnkForRelease = nullptr;

    switch (_formats[formatIdx].tymed) {
        case TYMED_HGLOBAL: {
            pmedium->hGlobal = makeHGlobalCopy(_mediums[formatIdx].hGlobal);
            break;
        }
        default: return DV_E_FORMATETC;
    }

    return S_OK;
}

HRESULT Win32DragDropObject::GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium) {
    return DATA_E_FORMATETC;
}

HRESULT Win32DragDropObject::QueryGetData(FORMATETC* pformatetc) {

    if (queryFormatIdx(pformatetc) >= 0i64) {
        return S_OK;
    }

    return DV_E_FORMATETC;
}

HRESULT Win32DragDropObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease) {
    return E_NOTIMPL;
}

HRESULT Win32DragDropObject::GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut) {
    pformatetcOut->ptd = nullptr;
    return E_NOTIMPL;
}

HRESULT Win32DragDropObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc) {

    if (dwDirection == DATADIR_SET) {
        return E_NOTIMPL;
    }

    return SHCreateStdEnumFmtEtc(_formats.size(), _formats.data(), ppenumFormatEtc);
}

HRESULT Win32DragDropObject::DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) {
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT Win32DragDropObject::DUnadvise(DWORD dwConnection) {
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT Win32DragDropObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise) {
    return OLE_E_ADVISENOTSUPPORTED;
}
