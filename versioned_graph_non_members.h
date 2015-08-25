#ifndef VERSIONED_GRAPH_NON_MEMBERS_H
#define VERSIONED_GRAPH_NON_MEMBERS_H

namespace boost {

template<typename graph_t>
auto add_vertex(versioned_graph<graph_t>& g){
    typedef typename versioned_graph<graph_t>::vertex_bundled bundled_type;
    return g.generate_vertex(bundled_type());
}

template<typename graph_t, typename propertyType>
auto add_vertex(const propertyType& p, versioned_graph<graph_t>& g){
    return g.generate_vertex(p);
}

template<typename graph_t, typename vertex_descriptor, typename propertyType>
auto add_edge(vertex_descriptor u,vertex_descriptor v,const propertyType& p, versioned_graph<graph_t>& g){
    return g.generate_edge(p,u,v);
}

template<typename graph_t,typename vertex_descriptor>
auto add_edge(vertex_descriptor u,vertex_descriptor v, versioned_graph<graph_t>& g){
    typedef typename versioned_graph<graph_t>::edge_bundled bundled_type;
    return g.generate_edge(bundled_type(),u,v);
}

template<typename graph_t,typename vertex_descriptor>
auto edge(vertex_descriptor u,vertex_descriptor v, const versioned_graph<graph_t>& g) {
    using namespace detail;
    typedef versioned_graph<graph_t> graph_type;
    auto p = boost::edge(u,v,dynamic_cast<const typename graph_type::graph_type&>(g));
    if(p.second){
        FILE_LOG(logDEBUG4) << "edge (" << u << ", " << v  <<  "): found existing intenally";
        auto edge_desc = p.first;
        FILE_LOG(logDEBUG4) << /*list.size()  <<*/ "records in history of edge";
        if(is_deleted(g.get_latest_revision(edge_desc))){
  FILE_LOG(logDEBUG4) << "edge: is deleted";
  return std::make_pair(typename graph_type::edge_descriptor(),false);
        }
        FILE_LOG(logDEBUG4) << "edge: not deleted";
    } else {
        FILE_LOG(logDEBUG4) << "edge: not found";
    }
    return p;
}

template<typename graph_t>
auto edges(const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;
    FILE_LOG(logDEBUG3) << "get edges";
    typename graph_type::edge_predicate predicate(&g);
    typename graph_type::edge_iterator iter_begin(predicate, g.edges_begin(), g.edges_end());
    typename graph_type::edge_iterator iter_end(predicate, g.edges_end(), g.edges_end());
    return std::make_pair(iter_begin,iter_end);
}

template<typename graph_t>
auto vertices(const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;

    typename graph_type::vertex_predicate predicate(&g);
    FILE_LOG(logDEBUG4) << "will create begin and end iterators for vertices";
    auto end = g.vertices_end();
    typename graph_type::vertex_iterator iter_begin(predicate, g.vertices_begin(), end);
    typename graph_type::vertex_iterator iter_end(predicate, end, end);
    FILE_LOG(logDEBUG4) << "created begin and end iterators for vertices";
    if(iter_begin!=iter_end){
        FILE_LOG(logDEBUG4) << "first vertex is: " << *iter_begin;
    } else {
        FILE_LOG(logDEBUG4) << "iterators are equal";
    }
    return std::make_pair(iter_begin,iter_end);
}

template<typename graph_t,typename vertex_descriptor>
void clear_out_edges(vertex_descriptor u, versioned_graph<graph_t
    >& g){
    typedef versioned_graph<graph_t
       > graph;
    auto ei = out_edges(u,g);
    for(typename graph::out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        remove_edge(*edge_iter,g);
    }
}

template<typename graph_t,typename vertex_descriptor>
void clear_in_edges(vertex_descriptor u, versioned_graph<graph_t
   >& g){
    typedef versioned_graph<graph_t
       > graph;
    auto ei = in_edges(u,g);
    for(typename graph::in_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        remove_edge(*edge_iter,g);
    }
}
template<typename graph_t, typename vertex_descriptor>
void clear_vertex(vertex_descriptor u, versioned_graph<graph_t
 >& g){
    clear_in_edges(u,g);
    clear_out_edges(u,g);
}

template<typename graph_t>
auto num_vertices(const versioned_graph<graph_t>& g){
    return g.num_vertices();
}

template<typename graph_t>
auto num_edges(const versioned_graph<graph_t>& g){
    return g.num_edges();
}

/*
template<typename T,typename U>
struct assert_same_types{
    typedef T A1;
    typedef U A2;
    static_assert(std::is_same<A1,A2>::value,"Types are not same");
    static void check() {}
};
template<typename From,typename To>
struct assert_convert_types{
    typedef From A1;
    typedef To A2;
    static_assert(std::is_convertible<A1,A2>::value,"Types are not convertible");
    static void check() {}
};
*/
template<typename graph_t, typename vertex_descriptor>
auto out_edges(vertex_descriptor u, const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;

    typename graph_type::edge_predicate predicate(&g);
    auto base_iter_p = out_edges(u,g.get_self());
    typename graph_type::out_edge_iterator iter_begin(predicate, base_iter_p.first, base_iter_p.second);
    typename graph_type::out_edge_iterator iter_end(predicate, base_iter_p.second, base_iter_p.second);
    return std::make_pair(iter_begin,iter_end);
}

template<typename graph_t, typename vertex_descriptor>
auto in_edges(vertex_descriptor u, const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;

    typename graph_type::edge_predicate predicate(&g);
    auto base_iter_p = in_edges(u,g.get_self());
    typename graph_type::in_edge_iterator iter_begin(predicate, base_iter_p.first, base_iter_p.second);
    typename graph_type::in_edge_iterator iter_end(predicate, base_iter_p.second, base_iter_p.second);
    return std::make_pair(iter_begin,iter_end);
}

template<typename graph_t, typename vertex_descriptor>
auto out_degree(vertex_descriptor u, const versioned_graph<graph_t>& g){
    return g.get_out_degree(u);
}

template<typename graph_t, typename vertex_descriptor>
auto in_degree(vertex_descriptor u, const versioned_graph<graph_t>& g){
    return g.get_in_degree(u);
}

template<typename graph_t, typename vertex_descriptor>
auto adjacent_vertices(vertex_descriptor u,
 const versioned_graph<graph_t
 >& g){
    typedef versioned_graph<graph_t> graph_type;

    auto adj = boost::adjacent_vertices(u,g.get_self());
    typename graph_type::adjacency_predicate predicate(&g,u);
    typename graph_type::adjacency_iterator iter_begin(predicate, adj.first, adj.second);
    typename graph_type::adjacency_iterator iter_end(predicate, adj.second, adj.second);
    return std::make_pair(iter_begin,iter_end);
}

template<typename graph_t, typename vertex_descriptor>
auto inv_adjacent_vertices(vertex_descriptor u, const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;

    auto adj = boost::inv_adjacent_vertices(u,g.get_self());
    typename graph_type::adjacency_predicate predicate(&g,u,true);
    typename graph_type::inv_adjacency_iterator iter_begin(predicate, adj.first, adj.second);
    typename graph_type::inv_adjacency_iterator iter_end(predicate, adj.second, adj.second);
    return std::make_pair(iter_begin,iter_end);
}

template<typename graph_t>
auto commit(versioned_graph<graph_t>& g){
    return g.commit();
}

template<typename graph_t>
auto undo_commit(versioned_graph<graph_t>& g){
    return g.undo_commit();
}

template<typename graph_t>
auto revert_changes(versioned_graph<graph_t>& g){
    return g.revert_uncommited();
}

template<typename graph_t, typename vertex_descriptor>
void remove_vertex(vertex_descriptor v, versioned_graph<graph_t>& g){
    g.set_deleted(v);
}

template<typename graph_t, typename vertex_descriptor>
void remove_edge(vertex_descriptor u,vertex_descriptor v, versioned_graph<graph_t>& g){
    auto p = edge(u,v,g);
    if(p.second){
        FILE_LOG(logDEBUG4) << "remove_edge: found existing";
        auto edge_desc = p.first;
        FILE_LOG(logDEBUG4) << "remove_edge: got desc: (" << boost::source(edge_desc,g) << ", " << boost::target(edge_desc,g) << ")";
        g.set_deleted(edge_desc);
    }
}

template<typename graph_t, typename edge_descriptor>
void remove_edge(edge_descriptor edge_desc, versioned_graph<graph_t>& g){
    FILE_LOG(logDEBUG4) << "remove_edge: (" << boost::source(edge_desc,g) << ", " << boost::target(edge_desc,g) << ")";
    g.set_deleted(edge_desc);
}

}


#endif // VERSIONED_GRAPH_NON_MEMBERS_H
