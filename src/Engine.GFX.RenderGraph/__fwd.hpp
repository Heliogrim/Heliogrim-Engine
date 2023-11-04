#pragma once

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
    class AnchorNode;
    class BarrierNode;
    class ConvergeNode;
    class DivergeNode;
    class SelectorNode;
    /**/
    class ProviderNode;
    class SubpassNode;
    /**/
    class CompileNode;
    class CompileSubpassNode;
    /**/
    class Auxiliary;
    class Visitor;
}
