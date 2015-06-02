#ifndef SUBGRAPH_H
#define SUBGRAPH_H


namespace boost{

    // All default interface for vf2_subgraph_iso
    template <typename GraphSmall,
              typename GraphLarge,
              typename SubGraphIsoMapCallback,
              typename EdgeEquivalencePredicate,
              typename VertexEquivalencePredicate>
    bool vf2_subgraph_iso(const GraphSmall& graph_small, const GraphLarge& graph_large,
                          SubGraphIsoMapCallback user_callback,
                          EdgeEquivalencePredicate edge_comp,
                          VertexEquivalencePredicate vertex_comp) {

      return vf2_subgraph_iso(graph_small, graph_large, user_callback,
                              get(vertex_index, graph_small), get(vertex_index, graph_large),
                              vertex_order_by_mult(graph_small),
                              edge_comp, vertex_comp);
    }
}

template<typename small_graph, typename large_graph>
struct attribute_equals{
    attribute_equals(const small_graph& s,const large_graph& l) : sgraph(s),lgraph(l){

    }
    template<typename descriptor1, typename descriptor2>
    bool operator()(descriptor1 desc1, descriptor2 desc2)const{
        return sgraph[desc1] == lgraph[desc2];
    }
    const small_graph& sgraph;
    const large_graph& lgraph;
};

template <typename small_graph, typename large_graph>
attribute_equals<small_graph,large_graph> make_attribute_equals_equivalent(const small_graph &g1,const large_graph& g2) {
    return attribute_equals<small_graph,large_graph>(g1,g2);
}

#endif // SUBGRAPH_H
