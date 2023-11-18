#pragma once

namespace hg::engine::gfx::render {
    class SubPass;
}

namespace hg::engine::gfx::render::graph {
    class CompileGraph;
    class RuntimeGraph;
    /**/
    class Builder;
    class BuilderVisitor;
    /**/
    class Component;
    class AnchorComponent;
    class BarrierComponent;
    /**/
    class ProviderComponent;
    class SubpassComponent;
    class SubpassAccelComponent;
    class SubpassInvocationComponent;
    /**/
    class CompileComponent;
    class CompileSubpassComponent;
    class CompileSubpassAccelComponent;
    class CompileSubpassSingleAccelComponent;
    class CompileSubpassMultiAccelComponent;
    class CompileSubpassMaterialAccelComponent;
    /**/
    class Node;
    class SLNode;

    class AnchorNode;
    class BarrierNode;
    class ConvergeNode;
    class DivergeNode;
    class SelectorNode;
    /**/
    class ProviderNode;
    class SubPassNodeBase;
    template <typename SubPassType_> requires _STD derived_from<SubPassType_, SubPass>
    class SubPassNode;
    /**/
    class CompileNode;
    class CompileSubPassNode;
    /**/
    class Auxiliary;
    class Visitor;
}
