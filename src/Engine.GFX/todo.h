#include <Engine.Common/Types.hpp>
#include "Shader/Prototype.hpp"
#include "Shader/Factory.hpp"
#include "Shader/ShaderStorage.hpp"

using namespace hg::engine::gfx;
using namespace hg;

void build_shader(cref<sptr<Device>> device_) {

    /**
     * General
     */
    shader::PrototypeBinding ubo {
        shader::BindingType::eUniformBuffer,
        1,
        shader::BindingUpdateInterval::ePerFrame,
        "ubo"
    };

    /**
     * Skybox
     */
    shader::PrototypeBinding skyboxCubeMap {
        shader::BindingType::eImageSampler,
        2,
        shader::BindingUpdateInterval::eUndefined,
        "cubemap"
    };

    /**
     * Terrain
     */
    shader::PrototypeBinding terrainHeight {
        shader::BindingType::eImageSampler,
        3,
        shader::BindingUpdateInterval::eUndefined,
        "heightmap"
    };

    shader::PrototypeBinding terrainBlend {
        shader::BindingType::eImageSampler,
        4,
        shader::BindingUpdateInterval::eUndefined,
        "blendmap"
    };

    /**
     * Material
     */
    shader::PrototypeBinding materialMeta {
        shader::BindingType::eStorageBuffer,
        6,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "materials"
    };

    shader::PrototypeBinding materialAlbedo {
        shader::BindingType::eImageSampler,
        7,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "albedoMap"
    };

    shader::PrototypeBinding materialNormal {
        shader::BindingType::eImageSampler,
        8,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "normalMap"
    };

    shader::PrototypeBinding materialMetalness {
        shader::BindingType::eImageSampler,
        9,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "metalnessMap"
    };

    shader::PrototypeBinding materialRoughness {
        shader::BindingType::eImageSampler,
        10,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "roughnessMap"
    };

    shader::PrototypeBinding materialAo {
        shader::BindingType::eImageSampler,
        11,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "aoMap"
    };

    shader::PrototypeBinding materialAlpha {
        shader::BindingType::eImageSampler,
        12,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "alphaMap"
    };

    shader::PrototypeBinding materialSpecular {
        shader::BindingType::eImageSampler,
        13,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "specularMap"
    };

    shader::PrototypeBinding materialDisplacement {
        shader::BindingType::eImageSampler,
        14,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "displaceMap"
    };

    /**
     * Mrt
     */
    shader::PrototypeBinding mrtUbo {
        shader::BindingType::eUniformBuffer,
        1,
        shader::BindingUpdateInterval::ePerFrame,
        "mrtUbo"
    };

    shader::PrototypeBinding mrtPosition {
        shader::BindingType::eImageSampler,
        2,
        shader::BindingUpdateInterval::ePerFrame,
        "mrtPosition"
    };

    shader::PrototypeBinding mrtNormal {
        shader::BindingType::eImageSampler,
        3,
        shader::BindingUpdateInterval::ePerFrame,
        "mrtNormal"
    };

    shader::PrototypeBinding mrtAlbedo {
        shader::BindingType::eImageSampler,
        4,
        shader::BindingUpdateInterval::ePerFrame,
        "mrtAlbedo"
    };

    shader::PrototypeBinding mrtMetalness {
        shader::BindingType::eImageSampler,
        5,
        shader::BindingUpdateInterval::ePerFrame,
        "mrtMetalness"
    };

    shader::PrototypeBinding mrtRoughness {
        shader::BindingType::eImageSampler,
        6,
        shader::BindingUpdateInterval::ePerFrame,
        "mrtRoughness"
    };

    shader::PrototypeBinding mrtAo {
        shader::BindingType::eImageSampler,
        7,
        shader::BindingUpdateInterval::ePerFrame,
        "mrtAo"
    };

    shader::PrototypeBinding mrtSpecular {
        shader::BindingType::eImageSampler,
        8,
        shader::BindingUpdateInterval::ePerFrame,
        "mrtSpecular"
    };

    shader::PrototypeBinding mrtDepth {
        shader::BindingType::eImageSampler,
        9,
        shader::BindingUpdateInterval::ePerFrame,
        "mrtDepth"
    };

    /**
     * Terrain
     */
    auto tvc = read_shader_file("resources/shader/terra.vert.spv");
    auto tfc = read_shader_file("resources/shader/terra.frag.spv");

    shader::Prototype terrainVertex { shader::ShaderType::eVertex, "terrain" };
    terrainVertex.add(ubo);
    //terrainVertex.add(terrainHeight);
    terrainVertex.storeCode(tvc.data(), static_cast<u64>(tvc.size()));

    shader::Prototype terraFrag { shader::ShaderType::eFragment, "terrain" };
    terraFrag.add(terrainBlend);
    terraFrag.add(materialMeta);
    terraFrag.add(materialAlbedo);
    terraFrag.add(materialNormal);
    terraFrag.add(materialMetalness);
    terraFrag.add(materialRoughness);
    terraFrag.add(materialAo);
    terraFrag.add(materialAlpha);
    terraFrag.add(materialSpecular);
    terraFrag.add(materialDisplacement);
    terraFrag.storeCode(tfc.data(), static_cast<u64>(tfc.size()));

    /**
     * Objects
     */
    auto ovc = read_shader_file("resources/shader/obj.vert.spv");
    auto ofc = read_shader_file("resources/shader/obj.frag.spv");

    shader::Prototype objVert { shader::ShaderType::eVertex, "object" };
    objVert.add(ubo);
    objVert.storeCode(ovc.data(), static_cast<u64>(ovc.size()));

    shader::Prototype objFragProt { shader::ShaderType::eFragment, "object" };
    objFragProt.add(materialMeta);
    objFragProt.add(materialAlbedo);
    objFragProt.add(materialNormal);
    objFragProt.add(materialMetalness);
    objFragProt.add(materialRoughness);
    objFragProt.add(materialAo);
    objFragProt.add(materialAlpha);
    objFragProt.add(materialSpecular);
    objFragProt.add(materialDisplacement);
    objFragProt.storeCode(ofc.data(), static_cast<u64>(ofc.size()));

    /**
     * Skybox
     */
    auto svc = read_shader_file("resources/shader/sky.vert.spv");
    auto sfc = read_shader_file("resources/shader/sky.frag.spv");

    shader::Prototype skyVertProt { shader::ShaderType::eVertex, "skybox" };
    skyVertProt.add(ubo);
    skyVertProt.storeCode(svc.data(), static_cast<u64>(svc.size()));

    shader::Prototype skyFragProt { shader::ShaderType::eFragment, "skybox" };
    skyFragProt.add(skyboxCubeMap);
    skyFragProt.storeCode(sfc.data(), static_cast<u64>(sfc.size()));

    /**
     * Gui
     */
    auto gvc = read_shader_file("resources/shader/gui.vert.spv");
    auto gfc = read_shader_file("resources/shader/gui.frag.spv");

    shader::Prototype guiVertProt { shader::ShaderType::eVertex, "gui" };
    guiVertProt.storeCode(gvc.data(), static_cast<u64>(gvc.size()));

    shader::Prototype guiFragProt { shader::ShaderType::eFragment, "gui" };
    guiFragProt.storeCode(gfc.data(), static_cast<u64>(gfc.size()));

    /**
     * Linking
     */
    auto lvc = read_shader_file("resources/shader/link.vert.spv");
    auto lfc = read_shader_file("resources/shader/link.frag.spv");

    shader::Prototype swapVertProt { shader::ShaderType::eVertex, "link" };
    swapVertProt.storeCode(lvc.data(), static_cast<u64>(lvc.size()));

    shader::Prototype swapFragProt { shader::ShaderType::eFragment, "link" };
    swapFragProt.add(mrtUbo);
    swapFragProt.add(mrtPosition);
    swapFragProt.add(mrtNormal);
    swapFragProt.add(mrtAlbedo);
    swapFragProt.add(mrtMetalness);
    swapFragProt.add(mrtRoughness);
    swapFragProt.add(mrtAo);
    swapFragProt.add(mrtSpecular);
    swapFragProt.add(mrtDepth);
    swapFragProt.storeCode(lfc.data(), static_cast<u64>(lfc.size()));

    /**
     * Build combined Mrt & Gui Prototypes
     */
    shader::Factory factory { device_ };
    auto result = factory.build(
        {
            terrainVertex,
            terraFrag,
            objVert,
            objFragProt,
            skyVertProt,
            skyFragProt,
            guiVertProt,
            guiFragProt
        }
    );

    ShaderStorage& storage = ShaderStorage::make();
    for (const auto& entry : result.shaders) {
        storage.store(entry);
    }

    /**
     * Build Linking Prototypes
     */
    result = factory.build(
        {
            swapVertProt,
            swapFragProt
        }
    );

    for (const auto& entry : result.shaders) {
        storage.store(entry);
    }

    #if FALSE
    /**
     * GraphicPass [GraphicPassMask::eDepthPass]
     */
    shader::PrototypeBinding depthPassUbo {
        shader::BindingType::eUniformBuffer,
        1,
        shader::BindingUpdateInterval::ePerFrame,
        "depthPassUbo"
    };

    /**
     * GraphicPass [GraphicPassMask::eDepthPass]
     */
    auto depthPassVertStatic = read_shader_file("resources/shader/depthpass_static.vert.spv");
    auto depthPassVertSkelet = read_shader_file("resources/shader/depthpass_skeletal.vert.spv");
    auto depthPassFrag = read_shader_file("resources/shader/depthpass.frag.spv");

    shader::Prototype depthPassVertStaticProt { shader::ShaderType::eVertex, "depthpass_static" };
    depthPassVertStaticProt.add(depthPassUbo);
    depthPassVertStaticProt.storeCode(depthPassVertStatic.data(), static_cast<u64>(depthPassVertStatic.size()));

    shader::Prototype depthPassVertSkeletProt { shader::ShaderType::eVertex, "depthpass_skeletal" };
    depthPassVertSkeletProt.add(depthPassUbo);
    depthPassVertSkeletProt.storeCode(depthPassVertStatic.data(), static_cast<u64>(depthPassVertStatic.size()));

    shader::Prototype depthPassFragProt { shader::ShaderType::eFragment, "depthpass" };
    depthPassFragProt.storeCode(depthPassFrag.data(), static_cast<u64>(depthPassFrag.size()));

    result = factory.build({
        depthPassVertStaticProt,
        depthPassVertSkeletProt,
        depthPassFragProt
    });

    for (const auto& entry : result.shaders) {
        storage.store(entry);
    }
    #endif

    #if FALSE
    /**
     * GraphicPass [GraphicPassMask::eFinalPass]
     */
    shader::PrototypeBinding finalPassUbo {
        shader::BindingType::eUniformBuffer,
        1,
        shader::BindingUpdateInterval::ePerFrame,
        "finalPassUbo"
    };

    shader::PrototypeBinding finalPassAlbedo {
        shader::BindingType::eImageSampler,
        2,
        shader::BindingUpdateInterval::ePerFrame,
        "mapped_albedo"
    };

    shader::PrototypeBinding finalPassNormal {
        shader::BindingType::eImageSampler,
        3,
        shader::BindingUpdateInterval::ePerFrame,
        "mapped_normal"
    };

    shader::PrototypeBinding finalPassPosition {
        shader::BindingType::eImageSampler,
        4,
        shader::BindingUpdateInterval::ePerFrame,
        "mapped_position"
    };

    shader::PrototypeBinding finalPassMrs {
        shader::BindingType::eImageSampler,
        5,
        shader::BindingUpdateInterval::ePerFrame,
        "mapped_mrs"
    };

    shader::PrototypeBinding finalPassDepth {
        shader::BindingType::eImageSampler,
        6,
        shader::BindingUpdateInterval::ePerFrame,
        "mapped_depth"
    };

    /**
     * GraphicPass [GraphicPassMask::eFinalPass]
     */
    auto finalPassVert = read_shader_file("resources/shader/finalpass.vert.spv");
    auto finalPassFrag = read_shader_file("resources/shader/finalpass.frag.spv");

    shader::Prototype finalPassVertProt { shader::ShaderType::eVertex, "finalpass" };
    finalPassVertProt.add(finalPassUbo);
    finalPassVertProt.storeCode(finalPassVert.data(), static_cast<u64>(finalPassVert.size()));

    shader::Prototype finalPassFragProt { shader::ShaderType::eFragment, "finalpass" };
    finalPassFragProt.add(finalPassUbo);
    finalPassFragProt.add(finalPassAlbedo);
    finalPassFragProt.add(finalPassNormal);
    finalPassFragProt.add(finalPassPosition);
    finalPassFragProt.add(finalPassMrs);
    finalPassFragProt.add(finalPassDepth);
    finalPassFragProt.storeCode(finalPassFrag.data(), static_cast<u64>(finalPassFrag.size()));

    result = factory.build({
        finalPassVertProt,
        finalPassFragProt
    });

    for (const auto& entry : result.shaders) {
        storage.store(entry);
    }
    #endif
}
