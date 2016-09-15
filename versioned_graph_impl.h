/***
 * author: Damian Lipka
 *
 * */

#ifndef VERSIONED_GRAPH_IMPL_H
#define VERSIONED_GRAPH_IMPL_H
#include <iostream>
namespace boost {

template<typename graph_t>
typename graph_traits<graph_t>::vertex_iterator versioned_graph<graph_t>::
vertices_begin() const {
    typename graph_traits<graph_t>::vertex_iterator iter = boost::vertices(get_base_graph()).first;
    return iter;
}

template<typename graph_t>
typename graph_traits<graph_t>::vertex_iterator versioned_graph<graph_t>::
vertices_end() const {
    typename graph_traits<graph_t>::vertex_iterator iter = boost::vertices(get_base_graph()).second;
    return iter;
}

template<typename graph_t>
typename graph_traits<graph_t>::edge_iterator versioned_graph<graph_t>::
edges_begin() const {
    typename graph_traits<graph_t>::edge_iterator iter = boost::edges(get_base_graph()).first;
    return iter;
}

template<typename graph_t>
typename graph_traits<graph_t>::edge_iterator versioned_graph<graph_t>::
edges_end() const {
    typename graph_traits<graph_t>::edge_iterator iter = boost::edges(get_base_graph()).second;
    return iter;
}

/**
 * Removes edge with history, operation cannot be undone
 * do not alter degree
 */
template<typename graph_t>
void versioned_graph<graph_t>::
remove_permanently(edge_descriptor e){
    edge_key key(e,*this);
    auto it = edges_history.find(key);
    assert(it!=edges_history.end());
    edges_history.erase(it);
    remove_edge(e,get_base_graph());
}

/**
 * Removes out_edge with history, operation cannot be undone
 * do not alter degree
 */
template<typename graph_t>
void versioned_graph<graph_t>::
remove_permanently(out_edge_iterator iter){
    edge_key key(*iter,*this);
    auto it = edges_history.find(key);
    assert(it!=edges_history.end());
    edges_history.erase(it);
    remove_edge(iter.base(),get_base_graph());
}

/**
 *  implementation of boost::remove_edge()
 */
template<typename graph_t>
void versioned_graph<graph_t>::
set_deleted(out_edge_iterator e){
    edges_history_type hist = get_history(*e);
    assert(!hist.empty());
    assert(!check_if_currently_deleted(*e));
    decr_degree(*e);
    if(hist.size()>1 || get_latest_revision(*e) < current_rev){
        // if there are many history records or was created in older revision then mark as deleted
        mark_deleted(*e,edge_bundled());
        assert(check_if_currently_deleted(*e));
    } else {
        remove_permanently(e);
    }
    --edge_count;
}

/**
 *  implementation of boost::remove_edge()
 */
template<typename graph_t>
void versioned_graph<graph_t>::
set_deleted(edge_descriptor e){
    edges_history_type hist = get_history(e);
    assert(!hist.empty());
    assert(!check_if_currently_deleted(e));
    decr_degree(e);
    if(hist.size()>1 || get_latest_revision(e) < current_rev){
        // if there are many history records or was created in older revision then mark as deleted
        mark_deleted(e,edge_bundled());
        assert(check_if_currently_deleted(e));
    } else {
        remove_permanently(e);
    }
    --edge_count;
}
/**
 *  Removes vertex with history, operation cannot be undone
 */
template<typename graph_t>
void versioned_graph<graph_t>::
remove_permanently(vertex_descriptor v){
    auto it = vertices_history.find(v);
    assert(it!=vertices_history.end());
    vertices_history.erase(it);
    assert(!versioned_graph<graph_t>::non_removable_vertex::value && !vertices_history.empty() && "vertex descriptors invalidated");
    remove_vertex(v,get_base_graph());
}

/**
 *  implementation of boost::remove_vertex()
 */
template<typename graph_t>
void versioned_graph<graph_t>::
set_deleted(vertex_descriptor v){
    if(get_history(v).size()>1 || get_latest_revision(v) < current_rev){
        mark_deleted(v,vertex_bundled());
        assert(check_if_currently_deleted(v));
    } else {
        remove_permanently(v);
    }
    --vertex_count;
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
    get_stored_data(v).decr_in_degree();
    if(std::is_same<directed_category,boost::undirected_tag>::value){
        get_stored_data(v).decr_out_degree();
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
    get_stored_data(v).incr_in_degree();
    if(std::is_same<directed_category,boost::undirected_tag>::value){
        get_stored_data(v).incr_out_degree();
    }
}

/**
 * remove latest record in history for edge,
 * if removed record made edge marked as deleted adjusts num_edges() result
 * do not alter edge attributes
 */
template<typename graph_t>
void versioned_graph<graph_t>::
clean_history( edges_history_type& hist, edge_descriptor desc){
    revision r = detail::get_revision(hist.top());
    hist.pop();
    if(is_deleted(r)){
        ++edge_count;
        incr_degree(desc);
    }
}

/**
 * remove latest record in history for vertex,
 * if removed record made vertex marked as deleted adjusts num_vertices() result
 * do not alter vertex attributes
 */
template<typename graph_t>
void versioned_graph<graph_t>::
clean_history( vertices_history_type& hist){
    revision rev_num = detail::get_revision(hist.top());
    if (is_deleted(rev_num)) {
       ++vertex_count;
       // vertex was marked as deleted, now will exist
    }
    hist.pop();
}

template<typename graph_t>
void versioned_graph<graph_t>::
clean_edges_to_current_rev(){
    typename graph_traits<graph_t>::edge_iterator ei, ei_end, next;
    boost::tie(ei,ei_end) = boost::edges(get_base_graph());
    std::deque<edge_descriptor> will_remove;
    for (next = ei; ei != ei_end; ei = next) {
        ++next;
        edge_descriptor e =  *ei;
        edges_history_type& hist = get_history(e);
        while(!hist.empty() && get_latest_revision(e)>=current_rev){
            clean_history(hist,e);
        }
        if(hist.empty()){
            will_remove.push_back(e);
        } else {
            (*this)[e] = property_handler<self_type,edge_descriptor,edge_bundled>::get_latest_bundled_value(e,*this);
        }
    }
    for(auto e : will_remove){
        decr_degree(e);
        remove_permanently(e);
        --edge_count;
    }
}

template<typename graph_t>
void versioned_graph<graph_t>::
clean_vertices_to_current_rev(){
    typename graph_traits<graph_t>::vertex_iterator vi, vi_end, next;
    std::deque<vertex_descriptor> will_remove;
    boost::tie(vi, vi_end) = vertices(get_base_graph());
    for (next = vi; vi != vi_end; vi = next) {
        ++next;
        vertex_descriptor v = *vi;
        vertices_history_type& hist = get_history(v);
        while(!hist.empty() && get_latest_revision(v)>=current_rev){
            clean_history(hist);
        }
        if(hist.empty()){
            // vertex was created in this or younger revision, we need to delete it
            will_remove.push_back(v);
        } else {
            (*this)[v] = property_handler<self_type,vertex_descriptor,vertex_bundled>::get_latest_bundled_value(v,*this);
        }
    }
    for(auto v : will_remove){
        remove_permanently(v);
        --vertex_count;
        // completly removed vertex history record
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
    // and copying its bundled property object.
    typename graph_t::vertex_iterator vi, vi_end;
    vertices_size_type v_count = 0;
    for (boost::tie(vi, vi_end) = vertices(g.get_base_graph()); vi != vi_end; ++vi) {
      vertex_descriptor v = add_vertex(get_base_graph());
      auto iter = g.vertices_history.find(*vi);
      assert(iter!=g.vertices_history.end());
      vertices_history.insert(std::make_pair(v,iter->second));
      (*this)[v] = g[*vi]; // set bundled properties
      if(!detail::is_deleted(detail::get_revision(iter->second.hist.top()))){
          ++v_count;
          // vertex marked as existing, so increase num_edges()
      }
      vertex_map[*vi] = v;
    }
    assert(v_count==vertex_count);
    // Copy the edges by adding each edge and copying its
    // property bundled object.
    typename graph_t::edge_iterator ei, ei_end;
    edges_size_type e_count = 0;
    for (boost::tie(ei, ei_end) = edges(g.get_base_graph()); ei != ei_end; ++ei) {
      edge_descriptor e;
      bool inserted;
      vertex_descriptor s = source(*ei,g), t = target(*ei,g);
      boost::tie(e, inserted) = add_edge(vertex_map[s],
                                         vertex_map[t], get_base_graph());
      assert(inserted);
      auto key = edge_key(*ei,g);
      auto iter = g.edges_history.find(key);
      assert(iter!=g.edges_history.end());
      auto newkey = edge_key(e,*this);
      edges_history.insert(std::make_pair(newkey,iter->second));
      (*this)[e] = g[key]; // set bundled properties

      revision r = detail::get_revision(iter->second.top());
      if(!is_deleted(r)){
          ++e_count;
          // edge marked as existing, so increase num_edges()
      }
    }
    // copy graph property
    (*this)[graph_bundle] = g[graph_bundle];

    BOOST_ASSERT_MSG(e_count==edge_count,("counted " + std::to_string(e_count) + "  edges while expected " + std::to_string(edge_count)).c_str());
    assert(boost::num_vertices(get_base_graph())==vertices_history.size());
    assert(boost::num_edges(get_base_graph())==edges_history.size());
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
}

/**
 *  init history structure for new edge
 */
template<typename graph_t>
void versioned_graph<graph_t>::init(edge_descriptor e,const edge_bundled& prop){
    edge_key key(e,*this);
    if(edges_history.find(key)==edges_history.end()){
        edges_history.insert(std::make_pair(key,edges_history_type()));
    }
    edges_history_type& list = get_history(e);
    assert(list.empty());
    list.push(detail::make_entry(current_rev,prop));
    incr_degree(e);
}

template<typename graph_t>
typename versioned_graph<graph_t>::vertex_descriptor
versioned_graph<graph_t>::generate_vertex(vertex_bundled prop){
    using namespace detail;
    vertex_descriptor v = boost::add_vertex(get_base_graph());
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
        auto key = edge_key(p.first,*this);
        init(key,prop);
        ++edge_count;
    }
    return p;
}

template<typename graph_t>
void versioned_graph<graph_t>::commit(){
    using namespace detail;
    auto ei = edges(*this);
    // copy properties from graph to history
    for(auto edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        edges_history_type& hist = get_history(*edge_iter);
        edge_bundled prop = (*this)[*edge_iter];
        if(hist.empty() || property_handler<self_type,edge_descriptor,edge_bundled>::is_update_needed(*edge_iter,*this,prop)){
            hist.push(make_entry(current_rev,prop));
        }
    }
    auto vi = boost::vertices(*this);
    for(auto vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
        vertices_history_type& hist = get_history(*vertex_iter);
        vertex_bundled prop = (*this)[*vertex_iter];
        if(hist.empty() || property_handler<self_type,vertex_descriptor,vertex_bundled>::is_update_needed(*vertex_iter,*this,prop)){
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
                hist.pop(); // clear history
            }
            // recreate single entry
            hist.push(make_entry(revision::create_start(),prop));
            assert(get_latest_revision(*edge_iter)==revision::create_start());
            if(is_deleted(old_rev)){ // edges marked as deleted,m should not exist after erasing history
                auto old = edge_iter;
                ++edge_iter; // avoid invalid iterator
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
            hist.pop(); // clear history
        }
        // recreate single entry
        hist.push(make_entry(revision::create_start(),prop));
        assert(get_latest_revision(*vertex_iter)==revision::create_start());
        if(is_deleted(old_rev)){ // vertices marked as deleted,m should not exist after erasing history
            auto old = vertex_iter;
            ++vertex_iter; // avoid invalid iterator
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
    clean_edges_to_current_rev();
    clean_vertices_to_current_rev();
    graph_bundled_history.clean_to_max(current_rev);
    (*this)[graph_bundle] = graph_bundled_history.get_latest();
}

}

#endif // VERSIONED_GRAPH_IMPL_H
