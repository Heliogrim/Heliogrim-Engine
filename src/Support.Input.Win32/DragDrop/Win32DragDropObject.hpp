#pragma once

#include <Engine.Common/stdafx.h>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Input/DragDrop/DragDropObject.hpp>

/**/
#include <objidl.h>

namespace hg::engine::input {
	struct DragDropObjectFilePayload;
	struct DragDropObjectTextPayload;

	using DragDropObjectPayload = Variant<Rc<DragDropObjectFilePayload>, Rc<DragDropObjectTextPayload>>;

	/**/

	class Win32DragDropObject :
		public DragDropObject,
		public IDataObject {
	public:
		using this_type = Win32DragDropObject;

	public:
		Win32DragDropObject();

		~Win32DragDropObject() override;

	private:
		void tidy();

	public:
		bool storeFiles(cref<Vector<string>> paths_) override;

		bool storeText(cref<string> text_) override;

	private:
		std::atomic_uint_fast32_t _useCount;

		DragDropObjectPayload _payload;

		Vector<FORMATETC> _formats;
		Vector<STGMEDIUM> _mediums;

	public:
		[[nodiscard]] u32 useCount() const noexcept;

	public:
		HRESULT QueryInterface(const IID& riid, void** ppvObject) override;

	public:
		ULONG AddRef() override;

		ULONG Release() override;

	private:
		s64 queryFormatIdx(ptr<FORMATETC> format_) const noexcept;

		[[nodiscard]] HGLOBAL makeHGlobalCopy(HGLOBAL src_);

	public:
		HRESULT GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium) override;

		HRESULT GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium) override;

		HRESULT QueryGetData(FORMATETC* pformatetc) override;

	public:
		HRESULT SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease) override;

	public:
		HRESULT GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut) override;

		HRESULT EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc) override;

	public:
		HRESULT DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) override;

		HRESULT DUnadvise(DWORD dwConnection) override;

		HRESULT EnumDAdvise(IEnumSTATDATA** ppenumAdvise) override;
	};
}
