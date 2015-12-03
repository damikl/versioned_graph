#ifndef VERSIONED_GRAPH_IMPL_H
#define VERSIONED_GRAPH_IMPL_H
namespace boost {

template<typename graph_t>
auto versioned_graph<graph_t>::
vertices_begin() const {
    typename graph_type::vertex_iterator iter = boost::vertices(get_base_graph()).first;
    FILE_LOG(logDEBUG4) << "fetched first vertex iterator";
    return iter;
}

template<typename graph_t>
auto versioned_graph<graph_t>::
vertices_end() const {
    typename graph_type::vertex_iterator iter = boost::vertices(get_base_graph()).second;
    FILE_LOG(logDEBUG4) << "fetched last vertex iterator";
    return iter;
}

template<typename graph_t>
auto versioned_graph<graph_t>::
edges_begin() const {
    typename graph_type::edge_iterator iter = boost::edges(get_base_graph()).first;
    FILE_LOG(logDEBUG4) << "fetched first edge iterator";
    return iter;
}

template<typename graph_t>
auto versioned_graph<graph_t>::
edges_end() const {
    typename graph_type::edge_iterator iter = boost::edges(get_base_graph()).second;
    FILE_LOG(logDEBUG4) << "fetched last edge iterator";
    return iter;
}

/**
 *  Removes edge with history, operation cannot be undone
 */
template<typename graph_t>
void versioned_graph<graph_t>::
remove_permanently(edge_descriptor e){
    FILE_LOG(logDEBUG3) << "remove edge completely";
    auto it = edges_history.find(e);
    assert(it!=edges_history.end());
    edges_history.erase(it);
    boost::remove_edge(e,*dynamic_cast<graph_type*>(this));
}

/**
 *  implementation of boost::remove_edge()
 */
template<typename graph_t>
void versioned_graph<graph_t>::
set_deleted(edge_descriptor e){
    FILE_LOG(logDEBUG3)     << "remove edge ( "
                            << boost::source(e,*this) << ", "
                            << boost::target(e,*this) << ")";
    edges_history_type hist = get_history(e);
    assert(!hist.empty());
    assert(!check_if_currently_deleted(e));
    decr_degree(e);
    if(hist.size()>1 || get_latest_revision(e) < current_rev){
        // if there are many history records or was created in older revision then mark as deleted
        FILE_LOG(logDEBUG3) << "remove edge: set as deleted";
        mark_deleted(e,edge_bundled());
        assert(check_if_currently_deleted(e));
    } else {
        remove_permanently(e);
    }
    --edge_count;
    FILE_LOG(logDEBUG3) << "set deleted: finnished";
}
/**
 *  Removes vertex with history, operation cannot be undone
 */
template<typename graph_t>
void versioned_graph<graph_t>::
remove_permanently(vertex_descriptor v){
    FILE_LOG(logDEBUG3) << "remove vertex completely";
    auto it = vertices_history.find(v);
    assert(it!=vertices_history.end());
    vertices_history.erase(it);
    boost::remove_vertex(v,*dynamic_cast<graph_type*>(this));
}

/**
 *  implementation of boost::remove_vertex()
 */
template<typename graph_t>
void versioned_graph<graph_t>::
set_deleted(vertex_descriptor v){
    if(get_history(v).size()>1 || get_latest_revision(v) < current_rev){
        FILE_LOG(logDEBUG3) << "remove vertex: set as deleted";
        mark_deleted(v,vertex_bundled());
        assert(check_if_currently_deleted(v));
    } else {
        remove_permanently(v);
    }
    --vertex_count;
    FILE_LOG(logDEBUG3) << "set vertex deleted: finnished";
}

/**
 * decrement out_degree and in_degree of graph
 */
template<typename graph_t>
void versioned_graph<graph_t>::
decr_degree(edge_descriptor e){
    vertex_descriptor u = boost::source(e,*this);
    vertex_descriptor v = boost::target(e,*this);
    get_stored_data(u).decr_out_degree();
    FILE_LOG(logDEBUG4) << "decrement out degree for " << u;

    get_stored_data(v).decr_in_degree();
    FILE_LOG(logDEBUG4) << "decrement in degree for " << v;
    if(std::is_same<directed_category,boost::undirected_tag>::value){
        get_stored_data(v).decr_out_degree();
        FILE_LOG(logDEBUG4) << "decrement out degree for " << v;
    }
}

/**
 * increment out_degree and in_degree of graph
 */
template<typename graph_t>
void versioned_graph<graph_t>::
incr_degree(edge_descriptor e){
    vertex_descriptor u = boost::source(e,*this);
    vertex_descriptor v = boost::target(e,*this);
    get_stored_data(u).incr_out_degree();
    FILE_LOG(logDEBUG4) << "increment out degree for " << u;

    get_stored_data(v).incr_in_degree();
    FILE_LOG(logDEBUG4) << "increment in degree for " << v;
    if(std::is_same<directed_category,boost::undirected_tag>::value){
        get_stored_data(v).incr_out_degree();
        FILE_LOG(logDEBUG4) << "increment out degree for " << v;
    }
}

template<typename graph_t>
void versioned_graph<graph_t>::
clean_history( edges_history_type& hist, edge_descriptor desc){
    revision r = detail::get_revision(hist.top());
    FILE_LOG(logDEBUG4) << "remove edges history ( containing " << hist.size() << " records ) entry: ( "
                        << boost::source(desc,*this) << ", "
                        << boost::target(desc,*this) << ") for rev: " << r;
    hist.pop();
    if(is_deleted(r)){
        ++edge_count;
        incr_degree(desc);
    }
    if(hist.empty()){
        FILE_LOG(logDEBUG4) << "after edge history removal: ( "
                            << boost::source(desc,*this) << ", "
                            << boost::target(desc,*this) << ") without any history records";
    } else {
        r = detail::get_revision(hist.top());
        FILE_LOG(logDEBUG4) << "after edge history removal: ( "
                            << boost::source(desc,*this) << ", "
                            << boost::target(desc,*this) << ") for rev: " << r;
    }

}

/**
 * remove latest record in history for that vertex,
 * if removed record made vertex marked as deleted adjusts num_vertices() result
 * do not alter vertex attributes
 */
template<typename graph_t>
void versioned_graph<graph_t>::
clean_history( vertices_history_type& hist, vertex_descriptor desc){
    revision rev_num = detail::get_revision(hist.top());
    if (is_deleted(rev_num)) {
       ++vertex_count;
    }
    FILE_LOG(logDEBUG4) << "remove vertices history containing (" << hist.size() << " records ) entry: ("<< desc << ") for rev: " << rev_num;
    hist.pop();
}

template<typename graph_t>
void versioned_graph<graph_t>::
clean_edges_to_current_rev(){
    FILE_LOG(logDEBUG) << "clean edges to rev " << current_rev;
    auto ei = boost::edges(get_base_graph());
    std::list<edge_descriptor> edges_to_be_removed;
    for(auto edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        FILE_LOG(logDEBUG3) << "clean edge:( "
                            << boost::source(*edge_iter,*this) << ", "
                            << boost::target(*edge_iter,*this) << ")";
        edges_history_type& hist = get_history(*edge_iter);
        while(!hist.empty() && get_latest_revision(*edge_iter)>=current_rev){
            clean_history(hist,*edge_iter);
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
        remove_permanently(e);
        --edge_count;
        FILE_LOG(logDEBUG3) << "completly removed edge history record";
    }
}

template<typename graph_t>
void versioned_graph<graph_t>::
clean_vertices_to_current_rev(){
    vertex_iterator vi, vi_end, next;

    boost::tie(vi, vi_end) = vertices(get_base_graph());
    for (next = vi; vi != vi_end; vi = next) {
        ++next;

        vertices_history_type& hist = get_history(*vi);
        FILE_LOG(logDEBUG3) << "clean history of vertex: " << *vi;
        while(!hist.empty() && get_latest_revision(*vi)>=current_rev){
            clean_history(hist,*vi);
        }
        if(hist.empty()){
            // vertex was created in this or younger revision, we need tu deleted it
            vertex_descriptor v = *vi;
            remove_permanently(v);
            --vertex_count;
            FILE_LOG(logDEBUG3) << "completly removed vertex history record";
        } else {
            (*this)[*vi] = property_handler<self_type,vertex_descriptor,vertex_bundled>::get_latest_bundled_value(*vi,*this);
            FILE_LOG(logDEBUG3) << "copied vertex propety";
        }
    }
}

template<typename graph_t>
versioned_graph<graph_t>::
versioned_graph(const versioned_graph& g ) : direct_base(0),
                                             graph_bundled_history(g.graph_bundled_history),
                                             vertex_count(g.vertex_count),
                                             edge_count(g.edge_count),
                                             current_rev(g.current_rev)
                                             {
    std::map<vertex_descriptor,vertex_descriptor> vertex_map;

    // Copy the stored vertex objects by adding each vertex
    // and copying its property object.
    vertex_iterator vi, vi_end;
    vertices_size_type v_count = 0;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
      vertex_descriptor v = add_vertex(get_base_graph());
      (*this)[v] = g[*vi];
      auto iter = g.vertices_history.find(*vi);
      assert(iter!=g.vertices_history.end());
      vertices_history.insert(std::make_pair(v,iter->second));
      if(!detail::is_deleted(detail::get_revision(iter->second.hist.top()))){
          ++v_count;
      }
      vertex_map[*vi] = v;
    }
    assert(v_count==vertex_count);
    // Copy the edges by adding each edge and copying its
    // property object.
    edge_iterator ei, ei_end;
    edges_size_type e_count = 0;
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
      edge_descriptor e;
      bool inserted;
      vertex_descriptor s = source(*ei,g), t = target(*ei,g);
      boost::tie(e, inserted) = add_edge(vertex_map[s],
                                         vertex_map[t], get_base_graph());
      assert(inserted);
      auto iter = g.edges_history.find(*ei);
      assert(iter!=g.edges_history.end());
      edges_history.insert(std::make_pair(e,iter->second));
      (*this)[e] = g[*ei];
      revision r = detail::get_revision(iter->second.top());

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
      r = detail::get_revision(new_hist.top());
      FILE_LOG(logDEBUG4) << "copy graph: new edge: ("
                          << boost::source(e,get_base_graph())
                          << ", "
                          << boost::target(e,get_base_graph())<< ") "
                          << new_hist.size() << " records in history, last rev: " << r;
    }
    (*this)[graph_bundle] = g[graph_bundle];

    BOOST_ASSERT_MSG(e_count==edge_count,("counted " + std::to_string(e_count) + "  edges while expected " + std::to_string(edge_count)).c_str());
    assert(boost::num_vertices(get_base_graph())==vertices_history.size());
    assert(boost::num_edges(get_base_graph())==edges_history.size());
    FILE_LOG(logDEBUG4) << "copied graph: " << vertex_count << "("<< vertices_history.size() << ") vertices, "
                                            << edge_count << "("<< edges_history.size() << ") edges";
}
/**
 *  init history structure for new vertex
 */
template<typename graph_t>
void versioned_graph<graph_t>::init(vertex_descriptor v,const vertex_bundled& prop){
    vertices_history.insert(std::make_pair(v,vertex_stored_data()));
    vertices_history_type& list = get_history(v);
    assert(list.empty());
    list.push(detail::make_entry(current_rev,prop));
    FILE_LOG(logDEBUG4) << "init vertex: " << v << " in rev " << current_rev;
}

/**
 *  init history structure for new edge
 */
template<typename graph_t>
void versioned_graph<graph_t>::init(edge_descriptor e,const edge_bundled& prop){
    if(edges_history.find(e)==edges_history.end()){
        edges_history.insert(std::make_pair(e,edges_history_type()));
    }
    edges_history_type& list = get_history(e);
    assert(list.empty());
    list.push(detail::make_entry(current_rev,prop));
    FILE_LOG(logDEBUG4) << "init edge: (" << source(e,*this) << "," << target(e,*this) << ") in rev " << current_rev;
    incr_degree(e);
}

template<typename graph_t>
typename versioned_graph<graph_t>::vertex_descriptor
versioned_graph<graph_t>::generate_vertex(vertex_bundled prop){
    using namespace detail;
    vertex_descriptor v = boost::add_vertex(get_base_graph());
    FILE_LOG(logDEBUG4) << "created vertex: " << v;
    (*this)[v] = prop;
    init(v);
    ++vertex_count;
    return v;
}

template<typename graph_t>
std::pair<typename versioned_graph<graph_t>::edge_descriptor,bool>
versioned_graph<graph_t>::
generate_edge(edge_bundled prop,vertex_descriptor u, vertex_descriptor v){
    using namespace detail;
    auto p = boost::add_edge(u,v,prop,get_base_graph());
    if(p.second){
        init(p.first,prop);
        ++edge_count;
        FILE_LOG(logDEBUG4) << "add edge (" << boost::source(p.first,get_base_graph()) << ", "
                                            << boost::target(p.first,get_base_graph())
                                            << ") with rev " << current_rev << " num of edges: " << edge_count;
    } else {
        FILE_LOG(logDEBUG4) << "edge (" << boost::source(p.first,get_base_graph()) << ", "
                                        << boost::target(p.first,get_base_graph()) << ") already exist";
    }
    return p;
}

template<typename graph_t>
void versioned_graph<graph_t>::commit(){
    using namespace detail;
    auto ei = edges(*this);
    for(auto edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        edges_history_type& hist = get_history(*edge_iter);
            edge_bundled prop = (*this)[*edge_iter];
            if(hist.empty() ||
                    property_handler<self_type,edge_descriptor,edge_bundled>::is_update_needed(*edge_iter,*this,prop)){
                hist.push(make_entry(current_rev,prop));
            }
    }
    auto vi = boost::vertices(*this);
    for(auto vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
        vertices_history_type& hist = get_history(*vertex_iter);
        vertex_bundled prop = (*this)[*vertex_iter];
        if(hist.empty() ||
                property_handler<self_type,vertex_descriptor,vertex_bundled>::is_update_needed(*vertex_iter,*this,prop)){
            hist.push(make_entry(current_rev,prop));
        }
    }
    graph_bundled_history.update_if_needed(current_rev,(*this)[graph_bundle]);
    ++current_rev;
}
/**
 * delete all history records and creates single new record for
 * latest graph property, each vertex and each edge, cannnot undo this operation
 */
template<typename graph_t>
void versioned_graph<graph_t>::erase_history(){
    using namespace detail;
    {
        auto ei = edges(get_base_graph());
        for(auto edge_iter = ei.first; edge_iter != ei.second; ) {
            edges_history_type& hist = get_history(*edge_iter);
            auto prop = property_handler<self_type,edge_descriptor,edge_bundled>::get_latest_bundled_value(*edge_iter,*this);
            const revision old_rev = get_latest_revision(*edge_iter);
            while(!hist.empty()){
                hist.pop();
            }
            hist.push(make_entry(revision::create_start(),prop));
            assert(get_latest_revision(*edge_iter)==revision::create_start());
            if(is_deleted(old_rev)){ // edges marked as deleted,m should not exist after erasing history
                auto old = edge_iter;
                ++edge_iter;
                remove_permanently(*old);
            } else {
                ++edge_iter;
            }
        }
    }
    auto vi = boost::vertices(get_base_graph());
    for(auto vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
        vertices_history_type& hist = get_history(*vertex_iter);
        auto prop = property_handler<self_type,vertex_descriptor,vertex_bundled>::get_latest_bundled_value(*vertex_iter,*this);
        const revision old_rev = get_latest_revision(*vertex_iter);
        while(!hist.empty()){
            hist.pop();
        }
        hist.push(make_entry(revision::create_start(),prop));
        assert(get_latest_revision(*vertex_iter)==revision::create_start());
        if(is_deleted(old_rev)){ // vertices marked as deleted,m should not exist after erasing history
            auto old = vertex_iter;
            ++vertex_iter;
            remove_permanently(*old);
        } else {
            ++vertex_iter;
        }
    }
    graph_bundled_history.clear();
    current_rev = revision::create_start();
}

template<typename graph_t>
void versioned_graph<graph_t>::
undo_commit(){
    if(current_rev.get_rev()>2){
        --current_rev;
    }
    FILE_LOG(logDEBUG) << "Undo commit";
    clean_edges_to_current_rev();
    clean_vertices_to_current_rev();
    graph_bundled_history.clean_to_max(current_rev);
    (*this)[graph_bundle] = graph_bundled_history.get_latest();
}

}

#endif // VERSIONED_GRAPH_IMPL_H
