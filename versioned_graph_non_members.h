/***
 * author: Damian Lipka
 *
 * */

#ifndef VERSIONED_GRAPH_NON_MEMBERS_H
#define VERSIONED_GRAPH_NON_MEMBERS_H
#include <list>

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
    return std::make_pair(typename graph_type::edge_descriptor(),false);
}

template<typename graph_t>
auto edges(const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;
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
    auto end = g.vertices_end();
    typename graph_type::vertex_iterator iter_begin(predicate, g.vertices_begin(), end);
    typename graph_type::vertex_iterator iter_end(predicate, end, end);
    return std::make_pair(iter_begin,iter_end);
}

template<typename graph_t,typename vertex_descriptor>
void clear_out_edges(vertex_descriptor u, versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;
    typedef typename graph_type::out_edge_iterator out_edge_iterator;
    out_edge_iterator ei, ei_end;
    tie(ei, ei_end) = out_edges(u,g);
    std::list<typename graph_type::edge_descriptor> to_remove;
    copy(ei, ei_end, std::back_inserter(to_remove));
    for(auto e : to_remove){
        g.set_deleted(e);
    }
}

template<typename graph_t,typename vertex_descriptor>
void clear_in_edges(vertex_descriptor u, versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;
    typedef typename graph_type::in_edge_iterator in_edge_iterator;
    in_edge_iterator ei, ei_end;
    boost::tie(ei, ei_end) = in_edges(u,g);
    std::list<typename graph_type::edge_descriptor> to_remove;
    copy(ei, ei_end, std::back_inserter(to_remove));
    for(auto e : to_remove){
        g.set_deleted(e);
    }
}

template<typename graph_t, typename vertex_descriptor>
void clear_vertex(vertex_descriptor u, versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;
    typedef typename graph_type::edge_iterator edge_iterator;
    edge_iterator ei, ei_end, next;
    std::list<typename graph_type::edge_descriptor> to_remove;
    boost::tie(ei, ei_end) = edges(g);
    for (next = ei; ei != ei_end; ei = next) {
      ++next;
      if (source(*ei,g)==u || target(*ei,g)==u){
        to_remove.push_back(*ei);
      }
    }
    for(auto e : to_remove){
        g.set_deleted(e);
    }
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
auto edge_range(vertex_descriptor u,vertex_descriptor v, const versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;

    typename graph_type::edge_predicate predicate(&g);
    auto base_iter_p = edge_range(u,v,g.get_base_graph());
    typename graph_type::out_edge_iterator iter_begin(predicate, base_iter_p.first, base_iter_p.first);
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
    typedef versioned_graph<graph_t> graph_type;
    typedef typename graph_type::out_edge_iterator out_edge_iterator;
    out_edge_iterator ei, ei_end, next;
    boost::tie(ei, ei_end) = out_edges(u,g);
    std::list<typename graph_type::edge_descriptor> to_remove;
    for (next = ei; ei != ei_end; ei = next) {
      ++next;
        if (target(*ei,g)==v){
            to_remove.push_back(*ei);
        }
    }
    for(auto e : to_remove){
        remove_edge(e, g);
    }
}

template<typename graph_t, typename edge>
void remove_edge(edge e, versioned_graph<graph_t>& g){
    g.set_deleted(e);
}

template <class predicate, typename vertex_descriptor, typename graph_t>
void remove_out_edge_if(vertex_descriptor u, predicate pred,
                        versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;
    typedef typename graph_type::out_edge_iterator out_edge_iterator;
    out_edge_iterator ei, ei_end, next;
    boost::tie(ei, ei_end) = out_edges(u,g);
    std::list<typename graph_type::edge_descriptor> to_remove;
    for (next = ei; ei != ei_end; ei = next) {
      ++next;
      if (pred(*ei)){
          to_remove.push_back(*ei);
      }
    }
    for(auto e : to_remove){
        remove_edge(e, g);
    }
}

template <class predicate, typename vertex_descriptor, typename graph_t>
void remove_in_edge_if(vertex_descriptor u, predicate pred,
                        versioned_graph<graph_t>& g){
    typedef versioned_graph<graph_t> graph_type;
    typedef typename graph_type::in_edge_iterator in_edge_iterator;
    in_edge_iterator ei, ei_end, next;
    boost::tie(ei, ei_end) = in_edges(u,g);
    std::list<typename graph_type::edge_descriptor> to_remove;
    for (next = ei; ei != ei_end; ei = next) {
      ++next;
      if (pred(*ei)){
          to_remove.push_back(*ei);
      }
    }
    for(auto e : to_remove){
        remove_edge(e, g);
    }
}

template <class predicate, typename graph_t>
void
remove_edge_if(predicate pred, versioned_graph<graph_t>& g)
{
  typedef versioned_graph<graph_t> graph_type;
  typedef typename graph_type::edge_iterator edge_iterator;
  edge_iterator ei, ei_end, next;
  boost::tie(ei, ei_end) = edges(g);
  std::list<typename graph_type::edge_descriptor> to_remove;
  for (next = ei; ei != ei_end; ei = next) {
    ++next;
    if (pred(*ei)){
        to_remove.push_back(*ei);
    }
  }
  for(auto e : to_remove){
      remove_edge(e, g);
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
