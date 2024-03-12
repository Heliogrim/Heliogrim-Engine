#include "Skybox.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Resource/FileResource.hpp>
#include <Engine.Scheduler/Async.hpp>

#include "../Command/CommandBuffer.hpp"
#include "../Geometry/Cube.hpp"
#include "../Texture/TextureFactory.hpp"

using namespace hg::engine::gfx;
using namespace hg;

Skybox::Skybox(cref<sptr<Device>> device_) :
	_device(device_) {

	Cube cube {};
	cube.scale() = math::fvec2 { 1.F };
	cube.segments() = math::uivec2 { 1uL };

	_geometry = cube.convert();

	_cubemap = TextureFactory::get()->build(
		{
			{ 1024u, 1024u, 1u },
			TextureFormat::eR8G8B8A8Unorm,
			1,
			TextureType::eCube,
			vk::ImageAspectFlagBits::eColor,
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::SharingMode::eExclusive
		}
	);

	TextureFactory::get()->buildView(_cubemap);

	auto pool = _device->graphicsQueue()->pool();
	pool->lck().acquire();
	CommandBuffer cmd = pool->make();
	cmd.begin();

	/**
	 * Transform Layout
	 */
	vk::ImageMemoryBarrier imb {
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		_cubemap.buffer().image(),
		{
			vk::ImageAspectFlagBits::eColor,
			0,
			_cubemap.mipLevels(),
			0,
			_cubemap.layer()
		}
	};

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlagBits::eAllCommands,
		vk::DependencyFlags(),
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imb
	);

	/**
	 *
	 */
	_cubemap.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	cmd.end();
	cmd.submitWait();
	cmd.release();

	pool->lck().release();

	const auto rootUrl = FileResource::getRootDir() + R"(\world\)";
	//const Url url { "file"sv, rootUrl + R"(skybox.ktx)" };

	::hg::todo_panic();
	/*
	auto src = loader::TextureLoader::get().load(url);
	src.finally([&, device_ = device_](Texture&& texture_) -> void {

	    auto pool = device_->transferQueue()->pool();

	    pool->lck().acquire();
	    CommandBuffer cmd = pool->make();

	    cmd.begin();

	    vk::ImageMemoryBarrier simb {
	        vk::AccessFlags(),
	        vk::AccessFlags(),
	        vk::ImageLayout::eShaderReadOnlyOptimal,
	        vk::ImageLayout::eTransferDstOptimal,
	        VK_QUEUE_FAMILY_IGNORED,
	        VK_QUEUE_FAMILY_IGNORED,
	        _cubemap.buffer().image(),
	        {
	            vk::ImageAspectFlagBits::eColor,
	            0,
	            _cubemap.mipLevels(),
	            0,
	            _cubemap.layer()
	        }
	    };

	    _cubemap.buffer()._vkLayout = vk::ImageLayout::eTransferDstOptimal;

	    cmd.vkCommandBuffer().pipelineBarrier(
	        vk::PipelineStageFlagBits::eAllCommands,
	        vk::PipelineStageFlagBits::eAllCommands,
	        vk::DependencyFlags(),
	        0, nullptr,
	        0, nullptr,
	        1, &simb
	    );

	    cmd.copyImage(texture_.buffer(), _cubemap.buffer(), {
	        {
	            vk::ImageAspectFlagBits::eColor,
	            0,
	            0,
	            6
	        },
	        {},
	        {
	            vk::ImageAspectFlagBits::eColor,
	            0,
	            0,
	            6
	        },
	        {},
	        {
	            MIN(texture_.width(), _cubemap.width()),
	            MIN(texture_.height(), _cubemap.height()),
	            1
	        }
	    });

	    vk::ImageMemoryBarrier eimb {
	        vk::AccessFlags(),
	        vk::AccessFlags(),
	        vk::ImageLayout::eTransferDstOptimal,
	        vk::ImageLayout::eShaderReadOnlyOptimal,
	        VK_QUEUE_FAMILY_IGNORED,
	        VK_QUEUE_FAMILY_IGNORED,
	        _cubemap.buffer().image(),
	        {
	            vk::ImageAspectFlagBits::eColor,
	            0,
	            _cubemap.mipLevels(),
	            0,
	            _cubemap.layer()
	        }
	    };

	    cmd.vkCommandBuffer().pipelineBarrier(
	        vk::PipelineStageFlagBits::eAllCommands,
	        vk::PipelineStageFlagBits::eAllCommands,
	        vk::DependencyFlags(),
	        0, nullptr,
	        0, nullptr,
	        1, &eimb
	    );

	    cmd.end();
	    cmd.submitWait();
	    cmd.release();

	    pool->lck().release();

	    _cubemap.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	    texture_.destroy();
	});

	scheduler::exec(scheduler::task::make_task(src));
	 */
}

#if FALSE
Skybox::Skybox(::std::nullptr_t) :
	Skybox() {

	/*
	{
		res::types::ResourceFile file (res::types::FileType::eTexture, FileResource::getRootDir() + std::string("\\world\\") + "skybox.ktx");
		const auto future = loader_->newLoadTexture(file);
		future.wait();
		_texture = future.get();

		_texture.makeSampler();
		_texture.makeView();
	}
	
	__internal::types::future<__TextureCubemap> tex = loader_->
		loadCubeMap(FileResource::getRootDir() + "\\world\\skybox.ktx");
	__internal::types::future<__Model> model = loader_->loadModel(
		FileResource::getRootDir() + "\\world\\skybox.obj");

	model.wait();
	_model = model.get();

	tex.wait();
	// _texture = tex.get();
	*/
	res::types::ResourceFile textureFile(res::types::eTexture, FileResource::getRootDir() + "\\world\\skybox.ktx");
	res::types::ResourceFile geometryFile(res::types::eAsset, FileResource::getRootDir() + "\\world\\skybox.obj");

	auto texture = loader::TextureLoader::get().load(textureFile);
	auto geometry = loader::GeometryLoader::get().load(geometryFile);

	// TODO: Make skybox loading
}
#endif

cref<Texture> Skybox::cubemap() const {
	return _cubemap;
}

ref<Texture> Skybox::cubemap() {
	return _cubemap;
}

ptr<const Mesh> Skybox::geometry() const {
	return _geometry;
}

ptr<const Mesh> Skybox::geometry() {
	return _geometry;
}
