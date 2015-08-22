#ifndef VERSIONED_GRAPH_IMPL_H
#define VERSIONED_GRAPH_IMPL_H
namespace boost {

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
vertices_begin() const {
    typename graph_type::vertex_iterator iter = boost::vertices(*dynamic_cast<const graph_type*>(this)).first;
    FILE_LOG(logDEBUG4) << "fetched first vertex iterator";
    return iter;
    //return typename graph_type::vertex_iterator(this->vertex_set().begin());
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
vertices_end() const {
    typename graph_type::vertex_iterator iter = boost::vertices(*dynamic_cast<const graph_type*>(this)).second;
    FILE_LOG(logDEBUG4) << "fetched last vertex iterator";
    return iter;
    //    return typename graph_type::vertex_iterator(this->vertex_set().end());
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
edges_begin() const {
    typename graph_type::edge_iterator iter = boost::edges(*dynamic_cast<const graph_type*>(this)).first;
    FILE_LOG(logDEBUG4) << "fetched first edge iterator";
    return iter;
    //    return typename graph_type::edge_iterator(this->m_edges.begin());
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
edges_end() const {
    typename graph_type::edge_iterator iter = boost::edges(*dynamic_cast<const graph_type*>(this)).second;
    FILE_LOG(logDEBUG4) << "fetched last edge iterator";
    return iter;
    //    return typename graph_type::edge_iterator(this->m_edges.end());
}


template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
void versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
set_deleted(edge_descriptor e){
    FILE_LOG(logDEBUG3)     << "remove edge ( "
                            << boost::source(e,*this) << ", "
                            << boost::target(e,*this) << ")";
    edges_history_type hist = get_history(e);
    assert(!hist.empty());
    assert(!check_if_currently_deleted(e));
    if(hist.size()>1 || get_latest_revision(e) < current_rev){
        decr_degree(e);
        set_deleted(e,edge_bundled());
        assert(check_if_currently_deleted(e));
    } else {
        auto it = edges_history.find(e);
        assert(it!=edges_history.end());
        edges_history.erase(it);
        boost::remove_edge(e,*dynamic_cast<graph_type*>(this));
    }
    --e_num;
    FILE_LOG(logDEBUG3) << "set deleted: finnished";
}
template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
void versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
set_deleted(vertex_descriptor v){
    if(get_history(v).size()>1 || get_latest_revision(v) < current_rev){
        set_deleted(v,vertex_bundled());
        assert(check_if_currently_deleted(v));
    } else {
        auto it = vertices_history.find(v);
        assert(it!=vertices_history.end());
        vertices_history.erase(it);
        boost::remove_vertex(v,*dynamic_cast<graph_type*>(this));
    }
    --v_num;
    FILE_LOG(logDEBUG3) << "set deleted: finnished";
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
void versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
decr_degree(edge_descriptor e){
    vertex_descriptor u = boost::source(e,*this);
    vertex_descriptor v = boost::target(e,*this);
    degree_size_type out_deg = get_stored_data(u).decr_out_degree();
    FILE_LOG(logDEBUG4) << "decrement out degree for " << u << " to " << out_deg;
    if(!std::is_same<directed_category,boost::directedS>::value){
        get_stored_data(v).decr_out_degree();
        FILE_LOG(logDEBUG4) << "decrement out degree for " << v << " to " << out_deg;
    }
    degree_size_type in_deg = get_stored_data(u).decr_in_degree();
    FILE_LOG(logDEBUG4) << "decrement in degree for " << u << " to " << in_deg;
    in_deg = get_stored_data(v).decr_in_degree();
    FILE_LOG(logDEBUG4) << "decrement in degree for " << v << " to " << in_deg;
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
void versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
incr_degree(edge_descriptor e){
    vertex_descriptor u = boost::source(e,*this);
    vertex_descriptor v = boost::target(e,*this);
    degree_size_type out_deg = get_stored_data(u).incr_out_degree();
    FILE_LOG(logDEBUG4) << "increment out degree for " << u << " to " << out_deg;
    if(!std::is_same<directed_category,boost::directedS>::value){
        out_deg = get_stored_data(v).incr_out_degree();
        FILE_LOG(logDEBUG4) << "increment out degree for " << v << " to " << out_deg;
    }
    degree_size_type in_deg = get_stored_data(u).incr_in_degree();
    FILE_LOG(logDEBUG4) << "increment in degree for " << u << " to " << in_deg;
    in_deg = get_stored_data(v).incr_in_degree();
    FILE_LOG(logDEBUG4) << "increment in degree for " << v << " to " << in_deg;
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
void versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
clean_history( edges_history_type& hist, edge_descriptor desc){
    revision r = get_revision(hist.front());
    FILE_LOG(logDEBUG4) << "remove edges history ( containing " << hist.size() << " records ) entry: ( "
                        << boost::source(desc,*this) << ", "
                        << boost::target(desc,*this) << ") for rev: " << r;
    hist.pop_front();
    if(is_deleted(r)){
        ++e_num;
        incr_degree(desc);
    }
    if(hist.empty()){
        FILE_LOG(logDEBUG4) << "after edge history removal: ( "
                            << boost::source(desc,*this) << ", "
                            << boost::target(desc,*this) << ") without any history records";
    } else {
        r = get_revision(hist.front());
        FILE_LOG(logDEBUG4) << "after edge history removal: ( "
                            << boost::source(desc,*this) << ", "
                            << boost::target(desc,*this) << ") for rev: " << r;
    }

}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
void versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
clean_history( vertices_history_type& hist, vertex_descriptor desc){
    revision r = get_revision(hist.front());
    if (is_deleted(r)) {
       ++v_num;
    }
    FILE_LOG(logDEBUG4) << "remove vertices history containing (" << hist.size() << " records ) entry: ("<< desc << ") for rev: " << r;
    hist.pop_front();
    r = get_revision(hist.front());
    FILE_LOG(logDEBUG4) << "after vertices history removal: ("<< desc << ") for rev: " << r;
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
void versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
clean_edges_to_current_rev(){
    FILE_LOG(logDEBUG) << "clean edges to rev " << current_rev;
    auto ei = boost::edges(get_self());
    std::list<edge_descriptor> edges_to_be_removed;
    for(auto edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        FILE_LOG(logDEBUG3) << "clean edge:( "
                            << boost::source(*edge_iter,*this) << ", "
                            << boost::target(*edge_iter,*this) << ")";
        edges_history_type& hist = get_history(*edge_iter);
        revision latest_rev = get_latest_revision(*edge_iter);
        while(!hist.empty() && latest_rev>=current_rev){
            FILE_LOG(logDEBUG3) << "clean history for rev: " << latest_rev;
            clean_history(hist,*edge_iter);
            latest_rev = get_latest_revision(*edge_iter);
        }
        if(hist.empty()){
            edges_to_be_removed.push_front(*edge_iter);
            decr_degree(*edge_iter);
        } else {
            (*this)[*edge_iter] = property_handler<self_type,edge_descriptor,edge_bundled>::get_latest_bundled_value(*edge_iter,*this);
            FILE_LOG(logDEBUG3) << "copied edge propety";
        }
    }
    for(edge_descriptor e : edges_to_be_removed){
        auto it = edges_history.find(e);
        assert(it!=edges_history.end());
        edges_history.erase(it);
        --e_num;
        FILE_LOG(logDEBUG3) << "completly removed edge history record";
        boost::remove_edge(e,get_self());
        FILE_LOG(logDEBUG3) << "removed edge from graph";
    }
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
void versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
clean_vertices_to_current_rev(){
    auto vi = boost::vertices(get_self());
    std::list<vertex_descriptor> vertices_to_be_removed;
    for(auto vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
        vertices_history_type& hist = get_history(*vertex_iter);
        FILE_LOG(logDEBUG3) << "clean history of vertex: " << *vertex_iter;
        while(!hist.empty() && get_latest_revision(*vertex_iter)>=current_rev){
            clean_history(hist,*vertex_iter);
        }
        if(hist.empty()){
            vertices_to_be_removed.push_front(*vertex_iter);
        } else {
            (*this)[*vertex_iter] = property_handler<self_type,vertex_descriptor,vertex_bundled>::get_latest_bundled_value(*vertex_iter,*this);
            FILE_LOG(logDEBUG3) << "copied vertex propety";
        }
    }
    for(vertex_descriptor v : vertices_to_be_removed){
        auto it = vertices_history.find(v);
        assert(it!=vertices_history.end());
        vertices_history.erase(it);
        --v_num;
        FILE_LOG(logDEBUG3) << "completly removed vertex history record";
        boost::remove_vertex(v,get_self());
        FILE_LOG(logDEBUG3) << "removed vertex from graph";
    }
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
versioned_graph(const versioned_graph& g ) : graph_type(),v_num(g.v_num),
                                             e_num(g.e_num),
                                             current_rev(g.current_rev) {
    // Would be better to have a constant time way to get from
    // vertices in x to the corresponding vertices in *this.
    std::map<vertex_descriptor,vertex_descriptor> vertex_map;

    // Copy the stored vertex objects by adding each vertex
    // and copying its property object.
    vertex_iterator vi, vi_end;
    vertices_size_type v_count = 0;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
      vertex_descriptor v = add_vertex(get_self());
      (*this)[v] = g[*vi];
      auto iter = g.vertices_history.find(*vi);
      assert(iter!=g.vertices_history.end());
      vertices_history.insert(std::make_pair(v,iter->second));
      if(!is_deleted(get_revision(iter->second.hist.front()))){
          ++v_count;
      }
      vertex_map[*vi] = v;
    }
    assert(v_count==v_num);
    // Copy the edges by adding each edge and copying its
    // property object.
    edge_iterator ei, ei_end;
    edges_size_type e_count = 0;
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
      edge_descriptor e;
      bool inserted;
      vertex_descriptor s = source(*ei,g), t = target(*ei,g);
      boost::tie(e, inserted) = add_edge(vertex_map[s],
                                         vertex_map[t], get_self());
      assert(inserted);
      auto iter = g.edges_history.find(*ei);
      assert(iter!=g.edges_history.end());
      edges_history.insert(std::make_pair(e,iter->second));
      (*this)[e] = g[*ei];
      revision r = get_revision(iter->second.front());

      if(!is_deleted(r)){
          ++e_count;
          FILE_LOG(logDEBUG4) << "copy graph: edge marked as existing";
      } else {
          FILE_LOG(logDEBUG4) << "copy graph: edge marked as deleted";
      }
      const edges_history_type& hist = g.get_history(*ei);
      FILE_LOG(logDEBUG4) << "copy graph: copy edge: ("
                          << boost::source(*ei,g)
                          << ", "
                          << boost::target(*ei,g)<< ") "
                          << hist.size() << " records in history, last rev: " << r;
      const edges_history_type& new_hist = get_history(e);
      r = get_revision(new_hist.front());
      FILE_LOG(logDEBUG4) << "copy graph: new edge: ("
                          << boost::source(e,get_self())
                          << ", "
                          << boost::target(e,get_self())<< ") "
                          << new_hist.size() << " records in history, last rev: " << r;
    }
    BOOST_ASSERT_MSG(e_count==e_num,("counted " + std::to_string(e_count) + "  edges while expected " + std::to_string(e_num)).c_str());
    assert(boost::num_vertices(get_self())==vertices_history.size());
    assert(boost::num_edges(get_self())==edges_history.size());
    FILE_LOG(logDEBUG4) << "copied graph: " << v_num << "("<< vertices_history.size() << ") vertices, "
                                            << e_num << "("<< edges_history.size() << ") edges";
}


template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
typename versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::vertex_descriptor
versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
generate_vertex(vertex_bundled prop){
    vertex_descriptor v = boost::add_vertex(get_self());
    FILE_LOG(logDEBUG4) << "created vertex: " << v;
    (*this)[v] = prop;
    vertices_history.insert(std::make_pair(v,vertex_stored_data()));
    vertices_history_type& list = get_history(v);
    assert(list.empty());
//    auto p = std::make_pair(current_rev,prop);
    list.push_front(make_entry(current_rev,prop));
    FILE_LOG(logDEBUG4) << "created vertex: " << v << " in rev " << current_rev;
    ++v_num;
    return v;
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
std::pair<typename versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::edge_descriptor,bool>
versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
generate_edge(edge_bundled prop,vertex_descriptor u, vertex_descriptor v){
    auto p = boost::add_edge(u,v,prop,get_self());
    if(p.second){
        if(edges_history.find(p.first)==edges_history.end()){
            edges_history.insert(std::make_pair(p.first,edges_history_type()));
        }
        edges_history_type& list = get_history(p.first);
        assert(list.empty());
        list.push_front(make_entry(current_rev,prop));
        incr_degree(p.first);
        ++e_num;
        FILE_LOG(logDEBUG4) << "add edge (" << boost::source(p.first,get_self()) << ", "
                                            << boost::target(p.first,get_self())
                                            << ") with rev " << current_rev << " num of edges: " << e_num;
    } else {
        FILE_LOG(logDEBUG4) << "edge (" << boost::source(p.first,get_self()) << ", "
                                        << boost::target(p.first,get_self()) << ") already exist";
    }
    return p;
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
void versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
commit(){
    auto ei = edges(*this);
    for(auto edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        edges_history_type& hist = get_history(*edge_iter);
            edge_bundled prop = (*this)[*edge_iter];
            if(hist.empty() ||
                    property_handler<self_type,edge_descriptor,edge_bundled>::is_update_needed(*edge_iter,*this,prop)){
                hist.push_front(make_entry(current_rev,prop));
            }
    }
    auto vi = boost::vertices(*this);
    for(auto vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
        vertices_history_type& hist = get_history(*vertex_iter);
        vertex_bundled prop = (*this)[*vertex_iter];
        if(hist.empty() ||
                property_handler<self_type,vertex_descriptor,vertex_bundled>::is_update_needed(*vertex_iter,*this,prop)){
            hist.push_front(make_entry(current_rev,prop));
        }
    }
    ++current_rev;
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
void versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>::
undo_commit(){
    --current_rev;
    FILE_LOG(logDEBUG) << "Undo commit";
    clean_edges_to_current_rev();
    clean_vertices_to_current_rev();
}

}

#endif // VERSIONED_GRAPH_IMPL_H