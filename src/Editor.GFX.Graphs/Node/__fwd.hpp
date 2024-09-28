#pragma once

namespace hg::editor::gfx::graph {
    class Node;

    class NodeInput;
    class NodeOutput;
    class NodeParameter;

    class NodeLink;

    struct NodeInputSpec;
    struct NodeOutputSpec;

    struct NodeInputValueAttributes;
    struct NodeOutputValueAttributes;

    enum class NodeLinkValueType;
    enum class NodeLinkTransportMode;

    enum class NodeInputConsumptionMode;
}

namespace hg::editor::gfx::graph::nodes {
    class ConstantInputNode;
    class TextureInputNode;
    class BufferInputNode;
    class UniformInputNode;

    class UniversePositionRuntimeInputNode;
    class UniverseNormalRuntimeInputNode;
    class TangentRuntimeInputNode;
    class VertexColorRuntimeInputNode;
    class InstanceIdRuntimeInputNode;
    class ScreenDepthRuntimeInputNode;
    class UvRuntimeInputNode;

    class ScalarAddMathNode;
    class ScalarSubMathNode;
    class ScalarMulMathNode;
    class ScalarDivMathNode;
    class ScalarSqrtMathNode;
    class ScalarPowMathNode;
    class ScalarLnMathNode;
    class ScalarSaturateMathNode;
    class ScalarInterpolateMathNode;

    class VectorAddMathNode;
    class VectorAddScalarMathNode;
    class VectorSubMathNode;
    class VectorDotMathNode;
    class VectorCrossMathNode;
    class VectorScaleMathNode;
    class VectorNormalizeMathNode;
    class VectorLengthMathNode;
    class VectorSaturateMathNode;
    class VectorInterpolateMathNode;

    class MatrixIdentityMathNode;
    class MatrixInverseMathNode;
    class MatrixTransposeMathNode;
    class MatrixMulMathNode;
    // TODO: MatrixDetMathNode ?

    class CosMathNode;
    class SinMathNode;
    class TanMathNode;
    class AcosMathNode;
    class AsinMathNode;
    class AtanMathNode;

    class MinMathNode;
    class MaxMathNode;

    class BitShiftLeftMathNode;
    class BitShiftRightMathNode;
    class BitOrMathNode;
    class BitXorMathNode;
    class BitAndMathNode;
    class BitNotMathNode;

    class SrgbNonLinearToLinearConversionNode;
    class GammaConversionNode;

    class GeneralBrdfOutputNode;
    class ClearCoatBrdfOutputNode;
    class ClothBrdfOutputNode;
    class BssrdfOutputNode;

    class SubGraphInputNode;
    class SubGraphOutputNode;
}
