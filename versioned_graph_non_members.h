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
    typename graph_type::out_edge_iterator it,end;
    tie(it,end) = out_edges(u,g);
    while(it!=end){
        if(target(*it,g)==v){
            return std::make_pair(*it,true);
        }
        ++it;
    }
    FILE_LOG(logDEBUG4) << "edge: not found";
    return std::make_pair(typename graph_type::edge_descriptor(),false);
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
std::pair<typename versioned_graph<graph_t>::vertex_iterator,
          typename versioned_graph<graph_t>::vertex_iterator>
vertices(const versioned_graph<graph_t>& g){
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
void clear_out_edges(vertex_descriptor u, versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph;
    std::list<typename graph::edge_descriptor> l;
    auto ei = out_edges(u,g);
    for(typename graph::out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        l.push_front(*edge_iter);
    }
    for(typename graph::edge_descriptor e : l){
        remove_edge(e,g);
    }
}

template<typename graph_t,typename vertex_descriptor>
void clear_in_edges(vertex_descriptor u, versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph;
    auto ei = in_edges(u,g);
    for(typename graph::in_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        remove_edge(*edge_iter,g);
    }
}

namespace detail {

template<typename category>
struct vertex_cleaner{
    template<typename graph_t, typename vertex_descriptor>
    static void clr_vertex(vertex_descriptor u, versioned_graph<graph_t>& g){
        boost::clear_in_edges(u,g);
        boost::clear_out_edges(u,g);
    }
};

template<>
struct vertex_cleaner<typename boost::directed_tag>{
    template<typename graph_t, typename vertex_descriptor>
    static void clr_vertex(vertex_descriptor u, versioned_graph<graph_t>& g){
        boost::clear_out_edges(u,g);
    }
};

}

template<typename graph_t, typename vertex_descriptor>
void clear_vertex(vertex_descriptor u, versioned_graph<graph_t>& g){
    detail::vertex_cleaner<typename versioned_graph<graph_t>::directed_category>::clr_vertex(u,g);
}

template<typename graph_t>
auto num_vertices(const versioned_graph<graph_t>& g){
    return g.num_vertices();
}

template<typename graph_t>
auto num_edges(const versioned_graph<graph_t>& g){
    return g.num_edges();
}

template<typename graph_t, typename vertex_descriptor>
auto out_edges(vertex_descriptor u, const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;

    typename graph_type::edge_predicate predicate(&g);
    auto base_iter_p = out_edges(u,g.get_base_graph());
    typename graph_type::out_edge_iterator iter_begin(predicate, base_iter_p.first, base_iter_p.second);
    typename graph_type::out_edge_iterator iter_end(predicate, base_iter_p.second, base_iter_p.second);
    return std::make_pair(iter_begin,iter_end);
}

template<typename graph_t, typename vertex_descriptor>
auto in_edges(vertex_descriptor u, const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;

    typename graph_type::edge_predicate predicate(&g);
    auto base_iter_p = in_edges(u,g.get_base_graph());
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
auto adjacent_vertices(vertex_descriptor u, const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;

    auto adj = boost::adjacent_vertices(u,g.get_base_graph());
    typename graph_type::adjacency_predicate predicate(&g,u);
    typename graph_type::adjacency_iterator iter_begin(predicate, adj.first, adj.second);
    typename graph_type::adjacency_iterator iter_end(predicate, adj.second, adj.second);
    return std::make_pair(iter_begin,iter_end);
}

template<typename graph_t, typename vertex_descriptor>
auto inv_adjacent_vertices(vertex_descriptor u, const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;

    auto adj = boost::inv_adjacent_vertices(u,g.get_base_graph());
    typename graph_type::inv_adjacency_predicate predicate(&g,u);
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

template<typename graph_t>
auto erase_history(versioned_graph<graph_t>& g){
    return g.erase_history();
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

template <class Predicate, typename vertex_descriptor, typename graph_t>
void remove_out_edge_if(vertex_descriptor u, Predicate pred,
                        versioned_graph<graph_t>& g){
    FILE_LOG(logDEBUG4) << "remove_out_edge_if";
    typedef versioned_graph<graph_t> graph_type;
    typedef typename graph_type::out_edge_iterator out_edge_iterator;
    out_edge_iterator ei, ei_end, next;
    boost::tie(ei, ei_end) = out_edges(u,g);
    for (next = ei; ei != ei_end; ei = next) {
      ++next;
      if (pred(*ei)){
        remove_edge(*ei, g);
      }
    }
    FILE_LOG(logDEBUG4) << "remove_out_edge_if ended";
}

template <class Predicate, typename vertex_descriptor, typename graph_t>
void remove_in_edge_if(vertex_descriptor u, Predicate pred,
                        versioned_graph<graph_t>& g){
    FILE_LOG(logDEBUG4) << "remove_in_edge_if";
    typedef versioned_graph<graph_t> graph_type;
    typedef typename graph_type::in_edge_iterator in_edge_iterator;
    in_edge_iterator ei, ei_end, next;
    boost::tie(ei, ei_end) = in_edges(u,g);
    for (next = ei; ei != ei_end; ei = next) {
      ++next;
      if (pred(*ei)){
        remove_edge(*ei, g);
      }
    }
    FILE_LOG(logDEBUG4) << "remove_in_edge_if ended";
}

template <class Predicate, typename graph_t>
void
remove_edge_if(Predicate pred, versioned_graph<graph_t>& g)
{
  typedef versioned_graph<graph_t> graph_type;
  typedef typename graph_type::edge_iterator edge_iterator;
  edge_iterator ei, ei_end, next;
  boost::tie(ei, ei_end) = edges(g);
  for (next = ei; ei != ei_end; ei = next) {
    ++next;
    if (pred(*ei))
      remove_edge(*ei, g);
  }
}

template<typename graph_t,typename vertex_size_type>
typename versioned_graph<graph_t>::vertex_descriptor
vertex(vertex_size_type n, const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;
    typename graph_type::vertex_iterator it,end;
    tie(it,end) = vertices(g);
    std::advance(it,n);
    return *it;
}

}




#endif // VERSIONED_GRAPH_NON_MEMBERS_H
