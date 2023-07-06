#pragma once

namespace hg::engine::gfx::render {
    class RenderGraph;
}

namespace hg::engine::gfx::render::graph {
    class Component;
    class AnchorComponent;
    class BarrierComponent;
    /**/
    class ProviderComponent;
    class SubpassComponent;
    class SubpassAccelComponent;
    /**/
    class Resolver;
    class ResolverOptions;
    class ResolverPass;
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
    class Auxiliary;
    class Visitor;
}
