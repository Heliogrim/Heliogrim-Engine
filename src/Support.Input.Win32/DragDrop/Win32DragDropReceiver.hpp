#pragma once

/**/
#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <Windows.h>
/**/
#include <ole2.h>
#include <Engine.Input/DragDrop/DragDropReceiver.hpp>

namespace hg::engine::input {
	class Win32DragDropReceiver :
		public DragDropReceiver,
		public IDropTarget {
	public:
		using this_type = Win32DragDropReceiver;

	public:
		Win32DragDropReceiver(const HWND hwnd_);

		~Win32DragDropReceiver() noexcept override;

	public:
		void setup() override;

		void destroy() override;

	private:
		HWND _hwnd;

	private:
		math::ivec2 _cursorPos;
		ptr<IDataObject> _data;
		bool _valid;

		std::atomic_uint_fast32_t _useCount;

	public:
		HRESULT QueryInterface(const IID& riid, void** ppvObject) override;

		ULONG AddRef() override;

		ULONG Release() override;

	private:
		[[nodiscard]] bool supportedDataObject(ptr<IDataObject> data_) const;

	public:
		HRESULT DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

		HRESULT DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

		HRESULT DragLeave() override;

		HRESULT Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
	};
}
