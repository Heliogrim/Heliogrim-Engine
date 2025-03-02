#pragma once

#include <functional>
#include <unordered_map>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

#if defined(ENV_MSVC) && defined(WIN32)
#define WIN32_LEAN_AND_MEAN
/**/
#include <Windows.h>
/* Note: Order dependent `windows header -> fileapi header` */
#include <fileapi.h>
/**/
#endif

#include "../File.hpp"

namespace hg::engine::res {
	class Watcher {
	public:
		using value_type = Watcher;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

	public:
		Watcher(cref<::hg::fs::File> root_);

		/**
		 * Copy Constructor
		 *
		 * @author Julius
		 * @date 23.09.2021
		 */
		Watcher(const_reference_type) = delete;

		/**
		 * Move Constructor
		 *
		 * @author Julius
		 * @date 23.09.2021
		 */
		Watcher(mref<value_type>) = delete;

		~Watcher();

	public:
		/**
		 * Copy Assignment operator
		 *
		 * @author Julius
		 * @date 23.09.2021
		 *
		 * @returns A shallow copy of this.
		 */
		reference_type operator=(const_reference_type) = delete;

		/**
		 * Move Assignment operator
		 *
		 * @author Julius
		 * @date 23.09.2021
		 *
		 * @returns A shallow copy of this.
		 */
		reference_type operator=(mref<value_type>) = delete;

	public:
		void tidy();

	private:
		void setup();

		void notify(bool publish_ = true);

	private:
		hg::fs::File _root;

	public:
		[[nodiscard]] cref<hg::fs::File> root() const noexcept;

		[[nodiscard]] hg::fs::File root() noexcept;

	public:
		[[nodiscard]] operator const hg::fs::File() const noexcept;

		[[nodiscard]] operator hg::fs::File() noexcept;

		#ifdef _WIN32

	private:
		HANDLE _handle;
		HANDLE _waitHandle;
		#endif

	private:
		std::unordered_map<hg::fs::File, std::filesystem::file_time_type> _state;

		std::function<void(hg::fs::File)> _createCallback;
		std::function<void(hg::fs::File)> _modifiedCallback;
		std::function<void(hg::fs::File)> _eraseCallback;

	public:
		void setCreateCallback(cref<std::function<void(hg::fs::File file_)>> callback_) noexcept;

		void setModifiedCallback(cref<std::function<void(hg::fs::File file_)>> callback_) noexcept;

		void setEraseCallback(cref<std::function<void(hg::fs::File file_)>> callback_) noexcept;
	};
}
