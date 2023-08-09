#pragma once
#include "Engine.Common/Make.hpp"
#include "Graph/NodeGraph.hpp"
#include "Node/__fwd.hpp"
#include "Node/NodeLink.hpp"
#include "Node/NodeOutput.hpp"
#include "Node/NodeInput.hpp"
#include "Node/Node.hpp"
#include "Node/Input/BufferInputNode.hpp"
#include "Node/Input/ConstantInputNode.hpp"
#include "Node/Input/SubGraphInputNode.hpp"
#include "Node/Math/ScalarPowMathNode.hpp"
#include "Node/Math/ScalarSubMathNode.hpp"
#include "Node/Math/VectorAddMathNode.hpp"
#include "Node/Math/VectorAddScalarMathNode.hpp"
#include "Node/Math/VectorScaleMathNode.hpp"
#include "Node/Output/SubGraphOutputNode.hpp"
#include "Graph/NodeGraph.hpp"

using namespace ::hg::editor::gfx::graph::nodes;
using namespace ::hg::editor::gfx::graph;

namespace hg::test {
    sptr<NodeGraph> mainFn(auto fn) {

        fn.SetWorldPosition(
            fn.GetLocalToWorld() * fn.GetLocalPosition()
        );

        fn.SetWorldNormal(
            fn.GetLocalToWorld() * fn.GetLocalNormal()
        );

        auto albedo = fn.SampleColor(
            fn.in.Next().TextureInput(),
            fn.GetLocalUv()
        );

        auto normal = fn.SampleNormal(
            fn.in.Next().TextureInput(),
            fn.GetLocalUv(),
            decltype(fn.SampleNormal)::eTangentSpace
        );

        auto roughness = fn.SampleColor(
            fn.in.Next().TextureInput(),
            fn.GetLocalUv()
        );

        auto metalness = fn.SampleColor(
            fn.in.Next().TextureInput(),
            fn.GetLocalUv()
        );

        auto sceneColor = fn.GeneralBrdf()
                            .SetAlbedo(albedo)
                            .SetNormal(normal)
                            .SetRoughness(roughness)
                            .SetMetalness(metalness);

        fn.out.Next().SetColor(sceneColor);
    }

    auto generalBrdfFn(auto fn) {

        auto albedo = fn.SampleColor(
            fn.in.Next().TextureInput(),
            fn.GetLocalUv()
        );
        auto normal = fn.Normalize(
            fn.SampleColor(
                fn.in.Next().TextureInput(),
                fn.GetLocalUv()
            )
        );
        auto position = fn.SampleData(
            fn.in.Next().TextureInput(),
            fn.GetLocalUv()
        );
        auto arm = fn.SampleData(
            fn.in.Next().TextureInput(),
            fn.GetLocalUv()
        );
        auto depth = fn.SampleData(
            fn.in.Next().TextureInput(),
            fn.GetLocalUv()
        ).Swizzle(decltype(fn.Swizzle)::eRRR);

        auto vd = fn.Normalize(fn.GetSceneViewPosition() - position);
        auto ndvd = fn.Saturate(fn.Dot(normal, vd));

        auto ref = fn.Reflect(fn.Negate(vd), normal);

        auto f0 = fn.Interpolate(fn.Vec3(0.04F), albedo, arm.r);
        auto f90 = fn.Saturate(fn.Multiply(f0.r, 50.0F));

        auto lights = fn.SampleBuffer(
            fn.in.Next().BufferInput()
        );
        auto Lo = fn.Accumulate(
            lights,
            [vd, position, normal, albedo, arm](auto fn, auto light) -> auto {

                auto ltp = fn.Normalize(light.position - position);
                auto h = fn.Normalize(vd - ltp);

                auto radiance = fn.Var(decltype(fn.Var)::eF32Vec3);
                auto ndl = fn.Var(decltype(fn.Var)::eF32);

                fn.If(
                    [light](auto fn) {
                        // TODO: light.limit > 0
                        return false;
                    }
                ).Then(
                    [light, radiance, ndl, normal, ltp](auto fn) {

                        fn.If(
                            [light](auto fn) {
                                // TODO: (light.direction.x + light.direction.y + light.direction.z) < 0.0001
                                return false;
                            }
                        ).Then(
                            [light, radiance](auto fn) {
                                //radiance = light_pointlight(light, position.xyz);
                                radiance = math::vec3 { 0.F };
                            }
                        ).Else(
                            [light, radiance](auto fn) {
                                //radiance = light_spotlight(light, position.xyz);
                                radiance = math::vec3 { 0.F };
                            }
                        );

                        ndl = fn.Saturate(fn.Dot(normal, ltp));
                    }
                ).Else(
                    [light, radiance, ndl, normal](auto fn) {

                        //radiance = light_directional(light, normal);
                        radiance = math::vec3 { 0.F };
                        ndl = fn.Saturate(fn.Dot(normal, fn.Normalize(light.direction)));

                    }
                );

                auto ndh = fn.Saturate(fn.Dot(normal, h));
                auto ldh = fn.Saturate(fn.Dot(ltp, h));
                auto hdvd = fn.Max(fn.Dot(h, vd), 0.0F);

                auto Lo = fn.Var(decltype(fn.Var)::eF32Vec3);

                fn.If(
                    [ndl](auto fn) {
                        // TODO: ndl > 0.0F
                        return false;
                    }
                ).Then(
                    [radiance, ndl, albedo, arm, Lo](auto fn) {

                        // TODO: auto D = ndf_ggx();
                        auto D = 0.F;
                        // TODO: auto G = geo_sch_smith();
                        auto G = 0.F;
                        // TODO: auto F = fresnel_sch();
                        auto F = math::vec3 { 0.F };

                        auto Fr = fn.Divide(D * G * F, 3.141F);

                        // TODO: auto Fd = disney_diff();
                        auto Fd = 0.F;

                        auto kD = fn.Subtract(1.F, F);
                        auto dc = fn.Multiply(albedo, fn.Subtract(1.F, arm.b));

                        Lo = fn.Multiply(dc * kD * Fd + Fr, radiance * ndl);
                    }
                );

                return Lo;
            }
        );

        // TODO: auto kS = fresnel_sch_rough();
        auto kS = math::vec3 { 0.F };
        auto kD = fn.Multiply(fn.Subtract(1.0F, kS), fn.Subtract(1.0F, arm.b));

        auto brdf = fn.SampleColor(
            fn.in.Next().TextureInput(),
            fn.Vec2(fn.Dot(normal, vd), arm.g)
        ).Swizzle(decltype(fn.Swizzle)::eRG);

        // TODO: auto preflec = prefilteredReflection();
        auto preflec = math::vec3 { 0.F };

        auto irradiance = fn.SampleColor(
            fn.in.Next().TextureInput(),
            normal
        );

        auto irrDiff = irradiance * albedo;
        auto rspec = preflec * (kS * brdf.x + brdf.y);

        auto ambient = (kD * irrDiff + rspec) * arm.r;
        auto color = ambient + Lo;

        fn.If(
            [depth](auto fn) {
                // TODO: Return depth.r >= 1.0
                return false;
            }
        ).Then(
            [color, albedo](auto fn) {
                color = albedo;
            }
        );

        // TODO: color = tonemap(color * EXPOSURE);
        color = color;
        // TODO: color = color * fn.Divide(1.0F, tonemap(fn.Vec3(11.2)));
        color = color * fn.Divide(1.0F, 1.0F);

        color = fn.Pow(color, fn.Vec3(1.0F / 2.2F));

        return fn.Vec4(color, 1.0F);
    }
}
