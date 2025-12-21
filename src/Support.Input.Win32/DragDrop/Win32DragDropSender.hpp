#pragma once

#include <Engine.Common/stdafx.h>
/**/
#include <ole2.h>
#include <Engine.Input/DragDrop/DragDropObject.hpp>
#include <Engine.Input/DragDrop/DragDropSender.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Task/SignaledQueue.hpp>

namespace hg::engine::input {
	class Win32DragDropSender :
		public DragDropSender,
		public IDropSource {
	public:
		using this_type = Win32DragDropSender;

	public:
		Win32DragDropSender();

		~Win32DragDropSender() noexcept override;

	public:
		void setup() override;

		void destroy() override;

	private:
		void dispatchDragDrop();

		void startDragDrop();

		void tidyDragDrop();

	public:
		void sendDragFiles(Vector<string> paths_) override;

		void sendDragText(cref<string> text_) override;

	private:
		DWORD _grfKeyState;
		ptr<DragDropObject> _cur;

		std::atomic_uint_fast32_t _useCount;

	public:
		HRESULT QueryInterface(const IID& riid, void** ppvObject_) override;

		ULONG AddRef() override;

		ULONG Release() override;

	public:
		HRESULT QueryContinueDrag(BOOL fEscapePressed_, DWORD grfKeyState_) override;

		HRESULT GiveFeedback(DWORD dwEffect_) override;
	};
}