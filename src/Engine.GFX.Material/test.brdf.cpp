#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/AccelerationStage.hpp>
#include <Engine.GFX.Acc/AccelerationStageOutput.hpp>
#include <Engine.GFX.Acc/AccelerationStageInput.hpp>
#include <Engine.GFX.Acc/__fwd.hpp>

#include "Material.hpp"
#include "MaterialPrototype.hpp"
#include "__fwd.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX/Geometry/Vertex.hpp>

#include "MaterialFactory.hpp"
#include "MaterialParameter.hpp"
#include "MaterialParameterStorage.hpp"
#include "MaterialPrototypeParameter.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg::engine::gfx::material;
using namespace hg;

void test_brdf_material() {

    auto stage0 = make_smr<AccelerationStage>(
        AccelerationStage {
            nullptr,
            AccelerationStageFlagBits::eVertex,
            {
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec3,
                    .token = AccelerationStageTransferToken::from("vertex/position")
                },
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec3,
                    .token = AccelerationStageTransferToken::from("vertex/uvm")
                },
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec3,
                    .token = AccelerationStageTransferToken::from("vertex/normal")
                },
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eBinding,
                    .dataType = AccelerationStageTransferDataType::eUniform,
                    .bindingMode = DataBindingMode::eDynamic,
                    .token = AccelerationStageTransferToken::from("camera")
                },
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eBinding,
                    .dataType = AccelerationStageTransferDataType::eUniform,
                    .bindingMode = DataBindingMode::eDynamic,
                    .token = AccelerationStageTransferToken::from("model")
                }
            },
            {
                AccelerationStageOutput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec4,
                    .token = AccelerationStageTransferToken::from("position")
                },
                AccelerationStageOutput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec4,
                    .token = AccelerationStageTransferToken::from("normal")
                },
                AccelerationStageOutput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec3,
                    .token = AccelerationStageTransferToken::from("uvm")
                }
            }
        }
    );

    auto stage1 = make_smr<AccelerationStage>(
        AccelerationStage {
            nullptr,
            AccelerationStageFlagBits::eFragment,
            {
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec4,
                    .token = AccelerationStageTransferToken::from("position")
                },
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec4,
                    .token = AccelerationStageTransferToken::from("normal")
                },
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec3,
                    .token = AccelerationStageTransferToken::from("uvm")
                },
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eBinding,
                    .dataType = AccelerationStageTransferDataType::eSampler,
                    .bindingMode = DataBindingMode::eStatic,
                    .token = AccelerationStageTransferToken::from("albedo")
                },
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eBinding,
                    .dataType = AccelerationStageTransferDataType::eSampler,
                    .bindingMode = DataBindingMode::eStatic,
                    .token = AccelerationStageTransferToken::from("normal")
                },
                AccelerationStageInput {
                    .transferType = AccelerationStageTransferType::eBinding,
                    .dataType = AccelerationStageTransferDataType::eSampler,
                    .bindingMode = DataBindingMode::eStatic,
                    .token = AccelerationStageTransferToken::from("arm")
                }
            },
            {
                AccelerationStageOutput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec3,
                    .token = AccelerationStageTransferToken::from("position")
                },
                AccelerationStageOutput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec3,
                    .token = AccelerationStageTransferToken::from("normal")
                },
                AccelerationStageOutput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec4,
                    .token = AccelerationStageTransferToken::from("albedo")
                },
                AccelerationStageOutput {
                    .transferType = AccelerationStageTransferType::eForward,
                    .dataType = AccelerationStageTransferDataType::eF32Vec3,
                    .token = AccelerationStageTransferToken::from("arm")
                },
            }
        }
    );

    Guid effectGuid {};
    GuidGenerate(effectGuid);

    AccelerationEffectInputs input {};
    AccelerationEffectBindings binding {};
    AccelerationEffectOutputs output {};

    {
        InputLayoutDescription ild {
            .token = AccelerationStageTransferToken::from("vertex"),
            .rate = DataInputRate::ePerInvocation,
            .attributes = {
                InputLayoutAttributeDescription {
                    .token = AccelerationStageTransferToken::from("position"),
                    .offset = offsetof(engine::gfx::vertex, position),
                    .size = sizeof(engine::gfx::vertex::position)
                },
                InputLayoutAttributeDescription {
                    .token = AccelerationStageTransferToken::from("normal"),
                    .offset = offsetof(engine::gfx::vertex, normal),
                    .size = sizeof(engine::gfx::vertex::normal)
                },
                InputLayoutAttributeDescription {
                    .token = AccelerationStageTransferToken::from("uvm"),
                    .offset = offsetof(engine::gfx::vertex, uvm),
                    .size = sizeof(engine::gfx::vertex::uvm)
                }
            }
        };

        input.addDescription(ild);
    }

    {
        BindingLayoutDescription camera {
            .token = AccelerationStageTransferToken::from("camera"),
            .dataType = AccelerationStageTransferDataType::eUniform,
            .bindingMode = DataBindingMode::eDynamic,
            .ioMode = DataIoMode::eRead,
            .attributes = {
                BindingLayoutAttributeDescription {
                    .token = AccelerationStageTransferToken::from("viewProj"),
                    .dataType = AccelerationStageTransferDataType::eF32Mat4x4, .offset = 0,
                    .size = sizeof(float) * 4 * 4
                }
            }
        };
        BindingLayoutDescription model {
            .token = AccelerationStageTransferToken::from("model"),
            .dataType = AccelerationStageTransferDataType::eUniform,
            .bindingMode = DataBindingMode::eDynamic,
            .ioMode = DataIoMode::eRead,
            .attributes = {
                BindingLayoutAttributeDescription {
                    .token = AccelerationStageTransferToken::from("model"),
                    .dataType = AccelerationStageTransferDataType::eF32Mat4x4, .offset = 0,
                    .size = sizeof(float) * 4 * 4
                }
            }
        };

        BindingLayoutDescription albedo {
            .token = AccelerationStageTransferToken::from("albedo"),
            .dataType = AccelerationStageTransferDataType::eSampler,
            .bindingMode = DataBindingMode::eStatic,
            .ioMode = DataIoMode::eRead,
            .attributes = {}
        };
        BindingLayoutDescription normal {
            .token = AccelerationStageTransferToken::from("normal"),
            .dataType = AccelerationStageTransferDataType::eSampler,
            .bindingMode = DataBindingMode::eStatic,
            .ioMode = DataIoMode::eRead,
            .attributes = {}
        };
        BindingLayoutDescription arm {
            .token = AccelerationStageTransferToken::from("arm"),
            .dataType = AccelerationStageTransferDataType::eSampler,
            .bindingMode = DataBindingMode::eStatic,
            .ioMode = DataIoMode::eRead,
            .attributes = {}
        };

        binding.addDescription(camera);
        binding.addDescription(model);
        binding.addDescription(albedo);
        binding.addDescription(normal);
        binding.addDescription(arm);
    }

    {
        OutputLayoutDescription position {
            .token = AccelerationStageTransferToken::from("position"),
            .rate = {},
            .attributes = {}
        };
        OutputLayoutDescription normal {
            .token = AccelerationStageTransferToken::from("normal"),
            .rate = {},
            .attributes = {}
        };
        OutputLayoutDescription albedo {
            .token = AccelerationStageTransferToken::from("albedo"),
            .rate = {},
            .attributes = {}
        };
        OutputLayoutDescription arm {
            .token = AccelerationStageTransferToken::from("arm"),
            .rate = {},
            .attributes = {}
        };

        output.addDescription(position);
        output.addDescription(normal);
        output.addDescription(albedo);
        output.addDescription(arm);
    }

    auto effect = make_smr<AccelerationEffect>(
        AccelerationEffect {
            _STD move(effectGuid),
            "Experimental PBR-BRDF Deferred Emitting",
            { stage0, stage1 },
            _STD move(input),
            _STD move(binding),
            _STD move(output)
        }
    );

    /**/

    MaterialFactory factory {};

    Guid matProtGuid {};
    GuidGenerate(matProtGuid);

    MaterialPrototype matProt {
        _STD move(matProtGuid),
        "Experimental PBR-BRDF Deferred Emitting",
        { effect },
        {}
    };

    {
        MaterialPrototypeParameter albedo = factory.buildMaterialPrototypeParameter(
            "albedo",
            AccelerationStageTransferDataType::eSampler
        );
        MaterialPrototypeParameter normal = factory.buildMaterialPrototypeParameter(
            "normal",
            AccelerationStageTransferDataType::eSampler
        );
        MaterialPrototypeParameter arm = factory.buildMaterialPrototypeParameter(
            "arm",
            AccelerationStageTransferDataType::eSampler
        );

        matProt.addParameter(_STD move(albedo));
        matProt.addParameter(_STD move(normal));
        matProt.addParameter(_STD move(arm));
    }

    // smr<Material> mat0 = matProt.instantiate();
    smr<Material> mat0 = factory.buildMaterial(make_smr<MaterialPrototype>(_STD move(matProt)));

    mat0->setParam("albedo"sv, smr<hg::engine::gfx::TextureResource> {});
    mat0->setParam("normal"sv, smr<hg::engine::gfx::TextureResource> {});
    mat0->setParam("arm"sv, smr<hg::engine::gfx::TextureResource> {});
}
