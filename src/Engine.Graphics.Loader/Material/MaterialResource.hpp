#pragma once

#include <Engine.Graphics.Material/Material.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace hg::engine::graphics {
	typedef resource::Resource<material::Material> MaterialResource;
}