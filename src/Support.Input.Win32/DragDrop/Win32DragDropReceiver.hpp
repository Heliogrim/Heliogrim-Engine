#pragma once

/**/
#define WIN32_LEAN_AND_MEAN
#ifndef STRICT
#define STRICT
#endif
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
		HRESULT STDMETHODCALLTYPE QueryInterface(
			REFIID riid_,
			_COM_Outptr_ void __RPC_FAR *__RPC_FAR * ppvObject_
		) override;

		ULONG STDMETHODCALLTYPE AddRef(void) override;

		ULONG STDMETHODCALLTYPE Release(void) override;

	private:
		[[nodiscard]] bool supportedDataObject(ptr<IDataObject> data_) const;

	public:
		HRESULT STDMETHODCALLTYPE DragEnter(
			__RPC__in_opt IDataObject* pDataObj_,
			DWORD grfKeyState_,
			POINTL pt_,
			__RPC__inout DWORD* pdwEffect_
		) override;

		HRESULT STDMETHODCALLTYPE DragOver(
			DWORD grfKeyState_,
			POINTL pt_,
			__RPC__inout DWORD* pdwEffect_
		) override;

		HRESULT STDMETHODCALLTYPE DragLeave(void) override;

		HRESULT STDMETHODCALLTYPE Drop(
			__RPC__in_opt IDataObject* pDataObj_,
			DWORD grfKeyState_,
			POINTL pt_,
			__RPC__inout DWORD* pdwEffect_
		) override;
	};
}
