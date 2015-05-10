#ifndef ARCHIVE_H
#define ARCHIVE_H
#include "iostream"
#include <limits>
#include <vector>
#include <algorithm>
#include "boost/graph/graph_traits.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/filtered_graph.hpp"
#include <boost/bimap.hpp>
#include <unordered_set>
#include "unordered_history_holder.h"
#include "key.h"
#include "mapping.h"

#include <boost/config.hpp>
#include <boost/graph/isomorphism.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/property_map/property_map.hpp>


template<typename T>
void print_to_log(T t){
    FILE_LOG(logDEBUG4) << t;
}

template<typename Graph, typename vertex_prop_type>
struct helper;

template<typename desc, typename vertex_mapping>
internal_vertex get_internal_vertex(const vertex_mapping& map,const desc& d){
    auto vertex_p = map.find(d);
    assert(vertex_p.second);
    return vertex_p.first->second;
}

template<typename vertex_container,typename vertex_mapping_type,typename vertex_container_iterator >
auto get_vertex_key(const vertex_container& vertices,const vertex_mapping_type& m, vertex_container_iterator it, revision r) {
    auto identifier = it->first;
    revision rev = vertices.get_revision(it,r);
    auto vertex_p = m.find(identifier);
    assert(vertex_p.second);
    auto vertex_desc = vertex_p.first->second;
    return create_vertex_id(vertex_desc,rev,identifier);
}

template<typename Graph>
class graph_archive
{
    typedef typename boost::vertex_bundle_type<Graph>::type vertex_properties;
    typedef typename boost::edge_bundle_type<Graph>::type edge_properties;
    typedef unordered_history_holder<vertex_properties,internal_vertex> vertices_container;
    typedef unordered_history_holder<edge_properties,std::pair<internal_vertex,internal_vertex> > edges_container;

    typedef typename boost::graph_traits<Graph>::edge_iterator edge_iterator;
    typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
    typedef vertex_id<typename Graph::vertex_descriptor> vertex_key;
    typedef edge_id<std::pair<typename Graph::vertex_descriptor,typename Graph::vertex_descriptor> > edge_key;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef mapping<internal_vertex,typename Graph::vertex_descriptor> vertex_mapping_type;
    typedef mapping<std::pair<internal_vertex,internal_vertex>,typename Graph::edge_descriptor> edge_mapping_type;
    typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;

    vertex_descriptor
    get_vertex_descriptor(const vertex_mapping_type& vm,
                        const internal_vertex& identifier
                                                )const{
        auto vertex_p = vm.find(identifier);
        assert(vertex_p.second);
        return vertex_p.first->second;
    }
    edge_descriptor
    get_edge_descriptor(const edge_mapping_type& em,
                        const std::pair<internal_vertex,internal_vertex>& identifiers_pair
                                                )const{
        auto edge_p = em.find(identifiers_pair);
        assert(edge_p.second);
        return edge_p.first->second;
    }
    std::pair<vertex_descriptor,vertex_descriptor>
    get_edges(const edge_mapping_type& em,
              const std::pair<internal_vertex,internal_vertex>& identifiers_pair,
              const Graph& g
            )const{
        edge_descriptor edge = get_edge_descriptor(em,identifiers_pair);
        vertex_descriptor source = boost::source(edge,g);
        vertex_descriptor target = boost::target(edge,g);
        return std::make_pair(source,target);
    }

    std::pair<internal_vertex,internal_vertex>
    get_edges(const vertex_mapping_type& m,
              const edge_descriptor& desc,
              const Graph& g
            ) const {
        vertex_descriptor source = boost::source(desc,g);
        vertex_descriptor target = boost::target(desc,g);
        return get_edges(m,std::make_pair(source,target));
    }

    std::pair<internal_vertex,internal_vertex>
    get_edges(const vertex_mapping_type& m,
              const std::pair<vertex_descriptor,vertex_descriptor>& desc
            )const{
        auto s_vertex_p = m.find(desc.first);
        assert(s_vertex_p.second);
        internal_vertex source_vertex = s_vertex_p.first->second;

        auto t_vertex_p = m.find(desc.second);
        assert(t_vertex_p.second);
        internal_vertex target_vertex = t_vertex_p.first->second;

        return std::make_pair(source_vertex,target_vertex);
    }

    template<typename iter >
    edge_key get_edge_key(const Graph& g, const edge_mapping_type& em, iter it,revision max_rev) const {
        std::pair<internal_vertex,internal_vertex> identifiers_pair = edges.get_key(it);
        revision rev = edges.get_revision(it,max_rev);

        edge_descriptor edge = get_edge_descriptor(em,identifiers_pair);
        vertex_descriptor source = boost::source(edge,g);
        vertex_descriptor target = boost::target(edge,g);
        return create_edge_id(std::make_pair(source,target),rev,identifiers_pair);
    }

public:
    friend struct helper<Graph,vertex_properties>;
    graph_archive() : _head_rev(0){
    }

 //   template<typename mapping_type>
    int commit(const Graph& g, vertex_mapping_type& mapping, edge_mapping_type& edge_mapping){
        std::pair<edge_iterator, edge_iterator> ei = boost::edges(g);
        std::pair<vertex_iterator, vertex_iterator> vi = boost::vertices(g);
        ++_head_rev;
#ifdef DEBUG
        FILE_LOG(logDEBUG1) << "before COMMIT " << head_rev() << std::endl;
        print_edges();
        print_vertices();
#endif

        {
            std::set<vertex_descriptor> ah_vertices; // vertices that are already here
            for(vertex_iterator vertex_iter = vi.first; vertex_iter != vi.second;++vertex_iter){
                auto ident = mapping.find(*vertex_iter);
                typename vertices_container::outer_const_iterator v_it;
                if(ident.second){
                    auto it = mapping.find(*vertex_iter).first;
                    internal_vertex v_identifier = it->second;
                    v_it = vertices.find(v_identifier);
                    assert(v_it!=vertices.end_full());
                    auto p = vertices.get_head_entry(v_it);
                    if(p.second){
                        typename vertices_container::inner_const_iterator iv_it = p.first;
                        if(helper<Graph,vertex_properties>::changed(*vertex_iter,iv_it,g)){
                            vertex_descriptor desc = *vertex_iter;
                            commit_vertex(g,v_identifier,desc,head_rev());
                        } else {
                            ah_vertices.insert(*vertex_iter);// note already existing vertex
                        }
                    }
                } else {
                    v_it = vertices.end_full();
                    vertex_descriptor desc = *vertex_iter;
                    internal_vertex vertex = internal_vertex::create();
                    commit_vertex(g,vertex,desc,head_rev());
                    mapping.insert(vertex,desc);
                }
            }
            // mark currently missing vertices as removed (negative revision)
            typename vertices_container::const_iterator v_it = vertices.cbegin(head_rev());
            while(v_it !=vertices.cend()){
               vertex_key curr = get_vertex_key(vertices,mapping,v_it,head_rev());
               FILE_LOG(logDEBUG1) << "vertex " << curr << " checked for deletion";
               if(curr.get_revision() < head_rev() && ah_vertices.find(curr.get_desc())== ah_vertices.end()) {
                    del_commit_vertex(curr.get_identifier(),head_rev());
               }
               ++v_it;
            }
        }
#ifdef DEBUG
        FILE_LOG(logDEBUG1) << "vertices commited";
        print_edges();
        print_vertices();
#endif
        {
            std::set<std::pair<vertex_descriptor,vertex_descriptor> > ah_edges; // edges that are already here
            for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
                auto edge = get_edges(mapping,*edge_iter,g);
                typename edges_container::outer_const_iterator e_it = edges.find(edge);
                if(e_it!=edges.end_full()){
                    // edge already exist, check for changes
                    std::pair<internal_vertex,internal_vertex> identifier = e_it->first;
                    auto p = edges.get_head_entry(e_it);
                    if(p.second){
                        typename edges_container::inner_const_iterator ie_it = p.first;
                        if(e_it == edges.end_full() || helper<Graph,edge_properties>::changed(*edge_iter,ie_it,g)){
                            commit_edge(g,identifier,*edge_iter,head_rev());
                        } else {
                            ah_edges.insert(std::make_pair(source(*edge_iter,g),target(*edge_iter,g)));
                        }
                    }
                } else {
                    // edge do not exist in archive yet
                    edge_descriptor desc = *edge_iter;
                    internal_vertex source = get_internal_vertex(mapping,boost::source(*edge_iter,g));
                    internal_vertex target = get_internal_vertex(mapping,boost::target(*edge_iter,g));
                    auto edge = std::make_pair(source,target);
                    commit_edge(g,edge,*edge_iter,head_rev());
                    edge_mapping.insert(edge,desc);
                }
            }
            // mark currently missing edges as removed (negative revision)
            typename edges_container::const_iterator e_it = edges.cbegin(head_rev());
            while(e_it !=edges.cend()){
                const edge_key curr = get_edge_key(g,edge_mapping,e_it,head_rev());
                FILE_LOG(logDEBUG1) << "commit, check if delete " << curr;
                auto edge = curr.get_desc();
                const int degree = std::max(boost::out_degree(edge.first,g),boost::out_degree(edge.second,g));
                if(curr.get_revision() < head_rev() && ah_edges.find(edge)== ah_edges.end() && degree==0 ) {
                    del_commit_edge(curr.get_identifier(),head_rev());
                }
                ++e_it;
            }
        }
#ifdef DEBUG
        FILE_LOG(logDEBUG1) << "after COMMIT " << head_rev();
        print_edges();
        print_vertices();
#endif
        return _head_rev;
    }
    /**
      check if edge exist in head revision
    **/
    bool contains_edge(const Graph& g, const typename boost::graph_traits<Graph>::edge_descriptor e) const{
        edge_key ed = edge_key::get_max(source(e, g),target(e, g));
        edge_key tmp = edges.lower_bound(ed)->first;
        return thesame(ed,tmp) && tmp.revision >=0;
    }
    /**
      check if vertex exist in head revision
    **/
    bool contains_vertex(const typename boost::graph_traits<Graph>::vertex_descriptor v)const{
        return contains_vertex(v,head_rev());
    }
    bool contains_vertex(const typename boost::graph_traits<Graph>::vertex_descriptor v, int revision)const{
        vertex_key ed = vertex_key::get_max(v);
        vertex_key tmp = vertices.lower_bound(ed)->first;
        return thesame(ed,tmp) && tmp.revision >=0;
    }
    int num_edges(int revision){
        return std::distance(edges.cbegin(revision),edges.cend());
    }
    int num_vertices(int revision){
        return std::distance(vertices.cbegin(revision),vertices.cend());
    }
    bool contains_edge(const edge_key& e) const{
        typename edges_container::const_iterator e_it = edges.lower_bound(e);
        if(e_it == edges.end())
            return false;
        return e_it->first.revision >=0;
    }
    void print_edges()const{
        FILE_LOG(logDEBUG3) << "print all edges:";
        for(auto it=edges.begin_full();it!=edges.end_full();++it){
        //    edge_key e = edges.get_key(it);
            std::pair<internal_vertex,internal_vertex> p = it->first;
            FILE_LOG(logDEBUG3) << p;
        }
        FILE_LOG(logDEBUG3) << "all edges printed";
        print_edges(head_rev());
    }
    void print_edges(int revision)const{
      //  std::cout << "edges in revision: " << revision << std::endl;
        FILE_LOG(logDEBUG2) << "edges in revision: " << revision;
        typename edges_container::const_iterator e_it = edges.cbegin(revision);
        while(e_it != edges.end()){
     //       edge_key curr_edge = edges.get_key(e_it);
         //   std::cout << "edge: " << curr_edge << std::endl;
            std::pair<internal_vertex,internal_vertex> p = e_it->first;
            FILE_LOG(logDEBUG2) << "edge: " << p;
            ++e_it;
        }
    }
    void print_vertices()const{
        FILE_LOG(logDEBUG3) << "print all vertices:";
        assert(std::distance(vertices.begin_full(),vertices.end_full())==vertices.size());
        for(auto it=vertices.begin_full();it!=vertices.end_full();++it){
            internal_vertex v = it->first;
            FILE_LOG(logDEBUG3) << v;
        }
        FILE_LOG(logDEBUG3) << "all vertices printed";
        print_vertices(head_rev());
    }
    void print_vertices(int revision)const{
      //  std::cout << "vertices in revision: " << revision << std::endl;
        FILE_LOG(logDEBUG2) << "vertices in revision: " << revision  << " size: " << vertices.size();
        typename vertices_container::const_iterator v_it = vertices.cbegin(revision);
        while(v_it != vertices.cend()){
            FILE_LOG(logDEBUG2) << "vertex: " << vertices.get_key(v_it);
            ++v_it;
        }
        FILE_LOG(logDEBUG2) << "Printing vertices finished";
    }
    Graph checkout(){
        return checkout(head_rev());
    }
    template<typename vertex_mapping_type, typename edge_mapping_type>
    Graph checkout(int rev, vertex_mapping_type vertex_mapping, edge_mapping_type edge_mapping) const {
        Graph n;
        revision r(rev);
        typename vertices_container::const_iterator v_it = vertices.cbegin(rev);
        if(v_it == vertices.cend()){
            FILE_LOG(logDEBUG2) << "No vertices to add";
        }
        while(v_it != vertices.cend()){
            internal_vertex vertex = v_it->first;
            vertex_descriptor d = boost::add_vertex(n);
            FILE_LOG(logDEBUG2) << "checkout, adding vertex: " << vertex << " as: " << d;
            vertex_mapping.insert(vertex,d);
            ++v_it;
        }
        typename edges_container::const_iterator e_it = edges.cbegin(rev);
        if(e_it == edges.cend()){
            FILE_LOG(logDEBUG2) << "No edges to add";
        }
        while(e_it != edges.cend()){
            std::pair<internal_vertex,internal_vertex> identifiers_pair = edges.get_key(e_it);
            revision edge_rev = edges.get_revision(e_it,r);
            vertex_descriptor source = get_vertex_descriptor(vertex_mapping,identifiers_pair.first);
            vertex_descriptor target = get_vertex_descriptor(vertex_mapping,identifiers_pair.second);

            if(!is_deleted(edge_rev)){
                auto e = boost::add_edge(source,target,n);
                assert(e.second);
                edge_descriptor desc = e.first;
#ifdef DEBUG
                std::cout << "checked out: " << source << " " << target << " rev: "<< edge_rev << " wanted: " << rev << std::endl;
                FILE_LOG(logDEBUG2) << "checked out: " << source << " " << target << " rev: "<< edge_rev << " wanted: " << rev;
#endif
 //               edges.set_edge_property(e_it,n,curr_edge);
                n[desc] = edges.get_property(e_it,edge_rev);
                edge_mapping.insert(identifiers_pair,desc);
            }else{
#ifdef DEBUG
                std::cout << "NOT checked out: " << source << " " << target << " rev: "<< edge_rev << " wanted: " << rev << std::endl;
                FILE_LOG(logDEBUG2)  << "NOT checked out: " << source << " " << target << " rev: "<< edge_rev << " wanted: " << rev;
#endif
            }
            ++e_it;
        }
//        correct_missing_vertices(n,rev,vertex_mapping);
        helper<Graph,vertex_properties>::set_vertex_properties(n,vertices,vertex_mapping, rev);
#ifdef DEBUG
        std::cout << "checkout of revision " << rev << " finished" << std::endl;
        FILE_LOG(logDEBUG2)  << "checked out of revision " << rev << " finished";
#endif
        return n;
    }
    void replace_with(Graph& g, int revision){
        typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
        {
     //       typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
            std::set<std::pair<vertex_descriptor,vertex_descriptor> > ah_edges; // edges that are already here
            edge_remover<graph_archive<Graph> > rm(revision,edges,*this);
            remove_edge_if(rm,g);
            std::pair<edge_iterator, edge_iterator> ei = boost::edges(g);
            for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
                typename edges_container::iterator e_it = lower_bound(*edge_iter,revision);
                if(edges.changed(*edge_iter,e_it,g)){
                    edges.set_edge_property(e_it,g,source(*edge_iter,g),target(*edge_iter,g));
                }
                ah_edges.insert(std::make_pair(source(*edge_iter,g),target(*edge_iter,g)));
            }
            // mark currently missing edges as removed (negative revision)
            typename edges_container::iterator e_it = edges.begin(revision);
            while(e_it !=edges.end()){
                edge_key curr = edges.get_key(e_it);
                curr.revision = revision;
                e_it = edges.lower_bound(curr);
                if(thesame(curr, edges.get_key(e_it))){
                    std::pair<vertex_descriptor,vertex_descriptor> edge = std::make_pair(curr.source,curr.target);
                    if(ah_edges.find(edge)== ah_edges.end()) {
                        add_edge(curr.source,curr.target,g);
                    }
                    e_it = edges.upper_bound(edge_key::get_min(curr.source,curr.target));
                }
            }
        }
        std::pair<vertex_iterator, vertex_iterator> vi = boost::vertices(g);
        {
            for(vertex_iterator vertex_iter = vi.first; vertex_iter != vi.second;++vertex_iter){
                typename vertices_container::iterator v_it = lower_bound(*vertex_iter,revision);
                if(v_it == vertices.end()){
                    remove_vertex(*vertex_iter,g);
                }
            }
            correct_missing_vertices(g,revision);
            helper<Graph,vertex_properties>::set_vertex_properties(g,vertices, revision);
        }
    }

    revision head_rev() const{
        return _head_rev;
    }

private:
    template<typename archive>
    class edge_remover{
        int revision;
        edges_container& edges;
        archive& g;
    public:
        edge_remover(int revision,edges_container& edges, archive& graph): revision(revision),edges(edges),g(graph) {}
        template<typename edge_desc>
        bool operator()(edge_desc e) const{
            return g.lower_bound(e,revision) == edges.end();
        }
    };

    typename edges_container::iterator
    lower_bound(const Graph& g, const typename boost::graph_traits<Graph>::edge_descriptor e) {
        return lower_bound(g,e,head_rev());
    }

    typename edges_container::iterator
    lower_bound(const Graph& g ,const typename boost::graph_traits<Graph>::edge_descriptor e, int revision) {
        edge_key ed = edge_key(std::make_pair(source(e, g),target(e, g)),revision);
        typename edges_container::iterator it = edges.lower_bound(ed);
        if(it == edges.end()){
            FILE_LOG(logDEBUG2) << "lower_bound found nothing";
            return it;
        }
        edge_key curr = edges.get_key(it);
        FILE_LOG(logDEBUG2) << "lower_bound found: " << curr << " , will check it if match " << ed;
        if(boost::source(e, g) != source(curr) ||
           boost::target(e, g) != target(curr) ||
           curr.is_deleted())
            return edges.end();
        return it;
    }

    typename vertices_container::iterator
    lower_bound(const typename boost::graph_traits<Graph>::vertex_descriptor v) {
        return lower_bound(v,head_rev());
    }
    typename vertices_container::iterator
    lower_bound(const typename boost::graph_traits<Graph>::vertex_descriptor v, int revision) {
        vertex_key max_key = vertex_key(v,revision);
        return lower_bound(v,max_key);
    }
    typename vertices_container::iterator
    lower_bound(const typename boost::graph_traits<Graph>::vertex_descriptor v, int revision, int identifier) {
        vertex_key max_key = vertex_key(v,revision,identifier);
        return lower_bound(v,max_key);
    }
    typename vertices_container::iterator
    lower_bound(const typename boost::graph_traits<Graph>::vertex_descriptor v, vertex_key key) {
        typename vertices_container::iterator it = vertices.lower_bound(key);
        if(it == vertices.end())
            return it;
        vertex_key curr = vertices.get_key(it);
        if(v != curr.get_desc() || curr.is_deleted())
            return vertices.end();
        return it;
    }

    void commit_vertex(const Graph&g,internal_vertex vertex, const vertex_descriptor& v, const revision& rev){
        FILE_LOG(logDEBUG2) << "COMMIT vertex: " << vertex;
        vertex_properties prop = helper<Graph,vertex_properties>::get_properties(g,v);
        vertices.insert(vertex,rev, prop);
    }
    void del_commit_vertex(const internal_vertex& v, revision rev){
        revision r = revision(-rev);
        FILE_LOG(logDEBUG2) << "COMMIT vertex delete: " << v << " with revision " << r;
        vertices.insert(v,r, vertex_properties());
    }
    void commit_edge(const Graph& g,const std::pair<internal_vertex,internal_vertex>& edge ,edge_descriptor v, revision rev){
        FILE_LOG(logDEBUG2) << "COMMIT edge: " << edge;
        edge_properties prop = helper<Graph,edge_properties>::get_properties(g,v);
        edges.insert(edge,rev, prop);
    }
    void del_commit_edge(std::pair<internal_vertex,internal_vertex> e,revision rev){
        revision r = revision(-rev);
        edges.insert(e,r, edge_properties());
    }

    template<typename mapping_type>
    void correct_missing_vertices(Graph& graph, int rev, mapping_type& map) const {
        typename graph_archive<Graph>::vertices_container::const_iterator v_it = vertices.cbegin(rev);
        FILE_LOG(logDEBUG2) << "adding missing vertices rev: " << rev;
  //     unsigned int v_size = boost::num_vertices(graph);
       unsigned int repo_size = 0;
        std::unordered_set<internal_vertex> already_known;
        while(v_it!=vertices.cend()){
            internal_vertex identifier = v_it->first;
            FILE_LOG(logDEBUG2) << vertices.get_key(v_it);
            already_known.insert(identifier);
            auto vertex_p = map.find(identifier);
            bool exist = vertex_p.second;
            if(!exist){
                FILE_LOG(logDEBUG2) << "adding missing vertex: " << identifier;
                vertex_descriptor d = boost::add_vertex(graph);
                map.insert(identifier,d);
            } else {
                vertex_descriptor desc = vertex_p.first->second;
                FILE_LOG(logDEBUG2) << "vertex: " << identifier << " not missing, found: " << desc;
            }
            ++repo_size;
            ++v_it;
        }/*
        if(repo_size <= v_size){
            FILE_LOG(logDEBUG2) << "no need to add vertices: repository: " << repo_size << " graph " << v_size;
            return;
        }

        FILE_LOG(logDEBUG2) << "start adding " << repo_size << " " << v_size << " vertices from rev: " << rev;
        for(unsigned int i = 0; i< repo_size - v_size;++i){
            vertex_descriptor d = boost::add_vertex(graph);
            map.insert(vertices.get_max_identifier(),d);
            FILE_LOG(logDEBUG2) << "ADD MISSING VERTEX " << d;
        }*/
        int num = boost::num_vertices(graph);
        MY_ASSERT(repo_size==num,std::to_string(repo_size) + " !=" + std::to_string(num));
    }

//    Graph& g;
    vertices_container vertices;
    edges_container edges;
    revision _head_rev;
};

template<typename Graph, typename property_type>
struct helper{
    template<typename vertex_mapping_type>
    static void set_vertex_properties(Graph& graph,
                                      const typename graph_archive<Graph>::vertices_container& vertices,
                                      const vertex_mapping_type& map,
                                      int rev){
#ifdef DEBUG
            FILE_LOG(logDEBUG3) << "started setting vertex properties for revision " << rev;
#endif
        typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
        std::pair<vertex_iterator,vertex_iterator> vi = boost::vertices(graph);
        revision r(rev);
        for(vertex_iterator v_it = vi.first; v_it != vi.second; ++v_it){
            typename graph_archive<Graph>::vertices_container::key_type key = get_internal_vertex(map,*v_it);;
#ifdef DEBUG
            FILE_LOG(logDEBUG3) << "setting vertex property " << *v_it << " rev: " << rev;
#endif
            auto res = vertices.get_entry(vertices.find(key),r);
            assert(res.second);
            typename graph_archive<Graph>::vertices_container::inner_const_iterator it = res.first;
#ifdef DEBUG
            FILE_LOG(logDEBUG3) << "found vertex key " << key << " in revision " << it->first << " and property: " << it->second;
#endif
            property_type p = it->second;
            graph[*v_it] = p;
        }
/*
        typename graph_archive<Graph>::vertices_container::const_iterator v_it = vertices.cbegin(rev);
        while(v_it != vertices.cend()){
            typename graph_archive<Graph>::vertex_key curr_vertex = v_it->first;
#ifdef DEBUG
            FILE_LOG(logDEBUG3) << "setting vertex property " << curr_vertex.id << " rev: " << curr_vertex.revision;
#endif
            if(curr_vertex.revision >=0){
                graph[curr_vertex.id] = v_it->second;
            }

            ++v_it;
#ifdef DEBUG
            FILE_LOG(logDEBUG1) << "vertex property in " << curr_vertex.id << " set ";
#endif
        }
*/
#ifdef DEBUG
            std::cout << "all vertex properties set" << std::endl;
#endif
    }
    template<typename descriptor>
    static property_type get_properties(const Graph& graph,descriptor v){
        return graph[v];
    }

    template<typename descriptor, typename iter>
    static bool changed(descriptor v, iter it,const Graph& g) {
        return g[v] != it->second;
    }
};

template<typename Graph>
struct helper<Graph,boost::no_property>{
    template<typename vertex_mapping_type>
    static void set_vertex_properties(Graph&,
                                      const typename graph_archive<Graph>::vertices_container&,
                                      const vertex_mapping_type&,
                                      int){
    }
    template<typename descriptor>
    static boost::no_property get_properties(const Graph& , descriptor ){
        return boost::no_property();
    }
    template<typename descriptor, typename iter>
    static bool changed(descriptor, iter,const Graph&) {
        return false;
    }
};

template<typename property_type>
inline bool compare(const property_type& p1,const property_type& p2){
    return p1 == p2;
}
template<>
inline bool compare(const boost::no_property&,const boost::no_property&){
    return true;
}
/**
  check if graphs have exactly same list of vertices and edges
**/
template<typename Graph>
bool equal(const Graph& g1, const Graph& g2){
    typedef typename boost::graph_traits<Graph>::edge_iterator edge_iterator;
    typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
//    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
//    typedef typename boost::edge_bundle_type<Graph>::type edge_properties;
    std::pair<edge_iterator, edge_iterator> e_g1 = boost::edges(g1);
    if(boost::num_edges(g1)!=boost::num_edges(g2)){
        FILE_LOG(logDEBUG1) << "GRAPH COMPARE: number of edges mismatch g1= " << boost::num_edges(g1) << " g2= " << boost::num_edges(g2) ;
        return false;
    }
    if(boost::num_vertices(g1)!=boost::num_vertices(g2)){
        FILE_LOG(logDEBUG1) << "GRAPH COMPARE: number of vertices mismatch g1= " << boost::num_vertices(g1) << " g2= " << boost::num_vertices(g2) ;
        return false;
    }
    for(edge_iterator it = e_g1.first; it!=e_g1.second;++it){
        std::pair<edge_descriptor,bool> p = boost::edge(source(*it,g1),target(*it,g1),g2);
        if(!p.second) {
            FILE_LOG(logDEBUG1) << "GRAPH COMPARE: edge " << source(*it,g1) << " " << target(*it,g1) << " not found";
            return false;
        } else
            if(!compare(g2[p.first],g1[*it])) {
                FILE_LOG(logDEBUG1) << "GRAPH COMPARE: edge " << source(*it,g1) << " " << target(*it,g1) << " != "
                                    << source(p.first,g2) << " " << target(p.first,g2);
                return false;
            }
    }
    std::pair<vertex_iterator, vertex_iterator> v_g1 = boost::vertices(g1);
    for(vertex_iterator it = v_g1.first; it!=v_g1.second;++it){
        if(!compare(g2[*it],g1[*it])){
            FILE_LOG(logDEBUG1) << "GRAPH COMPARE: vertex " << *it << " content do not match " << g2[*it] << " != " << g1[*it];
            return false;
        }
    }
    FILE_LOG(logDEBUG1) << "GRAPH COMPARE: match";
    return true;
}

enum vertex_desc_t { vertex_desc };
namespace boost {
    BOOST_INSTALL_PROPERTY(vertex, desc);
}

template<typename Graph>
class VertexIndexMap //maps vertex to index
{
public:
    typedef boost::readable_property_map_tag category;
    typedef int  value_type;
    typedef value_type reference;
    typedef typename Graph::vertex_descriptor key_type;

    VertexIndexMap(const mapping<internal_vertex,key_type>& m): map(m) {}
    VertexIndexMap(): map() {}
    mapping<internal_vertex,key_type> map;
    value_type get_value(key_type key) const{
        return map.find(key).first->second.get_identifier();
    }
};

template<typename UniquePairAssociativeContainer>
struct index_map : boost::associative_property_map< UniquePairAssociativeContainer >{
    index_map(){}
    index_map(UniquePairAssociativeContainer map): boost::associative_property_map< UniquePairAssociativeContainer >(m), m(map){
    }
    UniquePairAssociativeContainer m;
};

namespace boost {

template<typename Graph>
struct property_map<Graph, vertex_index_t > {
    typedef std::map<typename graph_traits<Graph>::vertex_descriptor, int> my_vertex_mapping;
    typedef index_map< my_vertex_mapping > const_type;
    //typedef type const_type ; //-- we do not define type as "vertex_index_t" map is read-only
};

template<typename Graph>
typename property_map<Graph, vertex_index_t >::const_type get(vertex_index_t, const Graph & g )
{
    typedef std::map<typename graph_traits<Graph>::vertex_descriptor, int> vertex_mapping;
    vertex_mapping v_indexes;
    index_map< vertex_mapping > v1_index_map(v_indexes);
    int id = 0;
    typename graph_traits<Graph>::vertex_iterator i, end;
    for (boost::tie(i, end) = vertices(g); i != end; ++i, ++id) {
        put(v1_index_map, *i, id);
    }
    return v1_index_map;
}


} //namespace boost

template<typename Graph>
bool check_isomorphism(const Graph& g1,const Graph& g2) {
    using namespace boost;
    int n = num_vertices(g1);
    if(boost::num_edges(g1)!=boost::num_edges(g2)){
        FILE_LOG(logDEBUG1) << "not isomorphic edges g1= " << boost::num_edges(g1) << " g2= " << boost::num_edges(g2) ;
        return false;
    }
    if(boost::num_vertices(g1)!=boost::num_vertices(g2)){
        FILE_LOG(logDEBUG1) << "not isomorphic, vertices g1= " << boost::num_vertices(g1) << " g2= " << boost::num_vertices(g2) ;
        return false;
    }
//    std::vector<typename graph_traits<Graph>::vertex_descriptor> v1(n), v2(n);

//    typedef std::map<typename graph_traits<Graph>::vertex_descriptor, int> vertex_mapping;
//    vertex_mapping v_indexes1,v_indexes2;
//    boost::associative_property_map< vertex_mapping >
//        v1_index_map(v_indexes1);
//    boost::associative_property_map< vertex_mapping >
//        v2_index_map(v_indexes2);

//    VertexIndexMap<Graph> v1_index_map(g1_mapping);
    typename property_map<Graph, vertex_index_t>::const_type
        v1_index_map = get(vertex_index, g1),
        v2_index_map = get(vertex_index, g2);
/*
    typename graph_traits<Graph>::vertex_iterator i, end;
    int id = 0;
    for (boost::tie(i, end) = vertices(g1); i != end; ++i, ++id) {
        put(v1_index_map, *i, id);
//        v1[id] = *i;
    }
    id = 0;
    for (boost::tie(i, end) = vertices(g2); i != end; ++i, ++id) {
        put(v2_index_map, *i, id);
//        v2[id] = *i;
    }
*/
    std::vector<typename graph_traits<Graph>::vertex_descriptor> f(n);
    auto iso_map = make_iterator_property_map(f.begin(), v1_index_map, f[0]);
    #if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
      bool ret = isomorphism
        (g1, g2, iso_map,
         degree_vertex_invariant(), get(vertex_index, g1), get(vertex_index, g2));
    #else
      bool ret = isomorphism(g1, g2, isomorphism_map(iso_map).vertex_index1_map(v1_index_map));
    #endif

    FILE_LOG(logDEBUG1) << "isomorphic? " << ret << "order: ";
    auto tmp = get(vertex_index, g2);
    for (std::size_t v = 0; v != f.size(); ++v){
        FILE_LOG(logDEBUG1) << "g2: " <<f[v] << " -> " <<get(tmp, f[v]);
        FILE_LOG(logDEBUG1) << "iso: " <<f[v] << " -> " <<get(iso_map, f[v]);
    }
    return ret;
}


#endif // ARCHIVE_H
