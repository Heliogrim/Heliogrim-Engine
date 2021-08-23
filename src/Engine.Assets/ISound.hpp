#pragma once
#include <Engine.Common/stdafx.h>

namespace clockwork::__internal::asset {
	class ISound {
	public:
		using identifier = _STD string;

		identifier getId() const;

		void setId(const identifier& id_);

		_STD string getFile() const;

		void setFile(const _STD string& file_);

		_STD string getMountPoint() const;

		void setMountPoint(const _STD string& point_);
		
	private:
		identifier _id;
		_STD string _file;
		_STD string _mountPoint;
	};

	TYPE_SHARED(clockwork::__internal::asset::ISound, ISound);
}