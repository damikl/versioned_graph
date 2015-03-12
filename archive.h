#ifndef ARCHIVE_H
#define ARCHIVE_H
#include "iostream"
#include <limits>
#include <vector>
#include <algorithm>
#include "boost/graph/graph_traits.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/filtered_graph.hpp"
#include "history_holder.h"
#include "key.h"

template<typename T>
void print_to_log(T t){
    FILE_LOG(logDEBUG4) << t;
}

template<typename Graph>
class graph_archive
{

    typedef typename boost::graph_traits<Graph>::edge_iterator edge_iterator;
    typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
    typedef key_id<typename Graph::vertex_descriptor> vertex_key;
    typedef key_id<std::pair<typename Graph::vertex_descriptor,typename Graph::vertex_descriptor> > edge_key;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
public:
    typedef typename boost::vertex_bundle_type<Graph>::type vertex_properties;
    typedef typename boost::edge_bundle_type<Graph>::type edge_properties;
    typedef history_holder<vertex_key,vertex_properties> vertices_container;
    typedef history_holder<edge_key,edge_properties> edges_container;

    friend struct helper<Graph,vertex_properties>;
    graph_archive() : _head_rev(0){
    }

    template<typename mapping_type>
    int commit(const Graph& g, mapping_type mapping){
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
                typename vertices_container::iterator v_it = lower_bound(*vertex_iter);
                if(v_it == vertices.end() || vertices.changed(*vertex_iter,v_it,g)){
                    commit(g,*vertex_iter,head_rev());// new vertex
                   }else{
                    ah_vertices.insert(*vertex_iter);// note already existing vertex
                }
            }
            // mark currently missing vertices as removed (negative revision)
            typename vertices_container::const_iterator v_it = vertices.cbegin(head_rev());
            while(v_it !=vertices.cend()){
               vertex_key curr = vertices.get_key(v_it);
               FILE_LOG(logDEBUG1) << "vertex " << curr << " checked for deletion";
               if(abs(curr.revision) < head_rev() && ah_vertices.find(curr.desc)== ah_vertices.end()) {
                    del_commit(curr.desc,-head_rev());
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
//            typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
            std::set<std::pair<vertex_descriptor,vertex_descriptor> > ah_edges; // edges that are already here
            for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
                typename edges_container::iterator e_it = lower_bound(g,*edge_iter);
                if(e_it == edges.end() || edges.changed(*edge_iter,e_it,g)){
                    commit(g,*edge_iter,head_rev());
                } else {
                    ah_edges.insert(std::make_pair(source(*edge_iter,g),target(*edge_iter,g)));
                }
            }
            // mark currently missing edges as removed (negative revision)
            typename edges_container::const_iterator e_it = edges.cbegin(head_rev());
            while(e_it !=edges.cend()){
                edge_key curr = edges.get_key(e_it);
                std::pair<vertex_descriptor,vertex_descriptor> edge = curr.desc;
                if(abs(curr.revision) < head_rev() && ah_edges.find(edge)== ah_edges.end()) {
                    del_commit(edge,-head_rev());
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
            edge_key e = edges.get_key(it);
            FILE_LOG(logDEBUG3) << e;
        }
        FILE_LOG(logDEBUG3) << "all edges printed";
        return print_edges(head_rev());
    }
    void print_edges(int revision)const{
      //  std::cout << "edges in revision: " << revision << std::endl;
        FILE_LOG(logDEBUG2) << "edges in revision: " << revision;
        typename edges_container::const_iterator e_it = edges.cbegin(revision);
        while(e_it != edges.end()){
            edge_key curr_edge = edges.get_key(e_it);
         //   std::cout << "edge: " << curr_edge << std::endl;
            FILE_LOG(logDEBUG2) << "edge: " << curr_edge;
            ++e_it;
        }
    }
    void print_vertices()const{
        FILE_LOG(logDEBUG3) << "print all vertices:";
        assert(std::distance(vertices.begin_full(),vertices.end_full())==vertices.size());
        for(auto it=vertices.begin_full();it!=vertices.end_full();++it){
            vertex_key v = vertices.get_key(it);
            FILE_LOG(logDEBUG3) << v;
        }
        FILE_LOG(logDEBUG3) << "all vertices printed";
        return print_vertices(head_rev());
    }
    void print_vertices(int revision)const{
      //  std::cout << "vertices in revision: " << revision << std::endl;
        FILE_LOG(logDEBUG2) << "vertices in revision: " << revision  << " size: " << vertices.size();
        typename vertices_container::const_iterator v_it = vertices.cbegin(revision);
        while(v_it != vertices.cend()){
            vertex_key curr_vertex = vertices.get_key(v_it);
         //   std::cout << "vertex: " << curr_vertex << std::endl;
            FILE_LOG(logDEBUG2) << "vertex: " << curr_vertex;
            ++v_it;
        }
        FILE_LOG(logDEBUG2) << "Printing vertices finished";
    }
    Graph checkout(){
        return checkout(head_rev());
    }
    template<typename mapping_type>
    Graph checkout(int rev, mapping_type mapping) const {
        Graph n;
        typename edges_container::const_iterator e_it = edges.begin(rev); // init with head revision
        while(e_it != edges.end()){
            edge_key curr_edge = edges.get_key(e_it);
            if(curr_edge.revision >=0){// negative revision == removed
                boost::add_edge(source(curr_edge),target(curr_edge),n);
#ifdef DEBUG
                std::cout << "checked out: " << source(curr_edge) << " " << target(curr_edge) << " rev: "<< curr_edge.revision << " wanted: " << rev << std::endl;
                FILE_LOG(logDEBUG2) << "checked out: " << source(curr_edge) << " " << target(curr_edge) << " rev: "<< curr_edge.revision << " wanted: " << rev;
#endif
                edges.set_edge_property(e_it,n,curr_edge);
            }else{
#ifdef DEBUG
                std::cout << "NOT checked out: " << source(curr_edge) << " " << target(curr_edge) << " rev: "<< curr_edge.revision << " wanted: " << rev << std::endl;
                FILE_LOG(logDEBUG2)  << "NOT checked out: " << source(curr_edge) << " " << target(curr_edge) << " rev: "<< curr_edge.revision << " wanted: " << rev;
#endif
            }
            ++e_it;
        }
        correct_missing_vertices(n,rev,mapping);
        helper<Graph,vertex_properties>::set_vertex_properties(n,vertices, rev);
#ifdef DEBUG
        std::cout << "checkout of revision " << rev << " finished" << std::endl;
        FILE_LOG(logDEBUG2)  << "checked out of revision revision " << rev << " finished";
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

    int head_rev() const{
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
        if(it == edges.end())
            return it;
        edge_key curr = edges.get_key(it);
        if(boost::source(e, g) != source(curr) ||
           boost::target(e, g) != target(curr) ||
           curr.revision < 0)
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
        typename vertices_container::iterator it = vertices.lower_bound(max_key);
        if(it == vertices.end())
            return it;
        vertex_key curr = vertices.get_key(it);
        if(v != curr.desc || curr.revision < 0)
            return vertices.end();
        return it;
    }

    vertex_key commit(const Graph&g, typename boost::graph_traits<Graph>::vertex_descriptor v, int revision){
 /*       typename std::map<vertex_descriptor,int>::const_iterator it = mapping.find(v);
        int ident = -1;
        if(it != mapping.end())
            ident = it->second;
 */       vertex_key el(v,revision/*,ident*/);
        FILE_LOG(logDEBUG2) << "COMMIT vertex: " << el;
        vertices.insert(el, helper<Graph,vertex_properties>::get_properties(g,v));
        return el;
    }
    vertex_key del_commit(typename boost::graph_traits<Graph>::vertex_descriptor v, int revision){
        assert(revision<0);
        vertex_key el(v,revision);
        FILE_LOG(logDEBUG2) << "COMMIT vertex delete: " << el;
        vertices.insert(el, vertex_properties());
        return el;
    }
    edge_key commit(const Graph& g, typename boost::graph_traits<Graph>::edge_descriptor v, int revision){
        edge_key el(std::make_pair(source(v, g),target(v, g)),revision);
        FILE_LOG(logDEBUG2) << "COMMIT edge: " << el;
        edges.insert(el, helper<Graph,edge_properties>::get_properties(g,v));
        return el;
    }
    edge_key del_commit(std::pair<  typename boost::graph_traits<Graph>::vertex_descriptor,
                                typename boost::graph_traits<Graph>::vertex_descriptor> v,
                    int revision){
        assert(revision<0);
        edge_key el(std::make_pair(v.first,v.second),revision);
        FILE_LOG(logDEBUG2) << "COMMIT edge delete: " << el;
        edges.insert(el, edge_properties());
        return el;
    }

    template<typename mapping_type>
    void correct_missing_vertices(Graph& graph, int rev, mapping_type& map) const {
        typename graph_archive<Graph>::vertices_container::const_iterator v_it = vertices.cbegin(rev);
        FILE_LOG(logDEBUG2) << "adding missing vertices rev: " << rev;
        unsigned int repo_size = std::distance(v_it,vertices.cend());
        unsigned int v_size = boost::num_vertices(graph);
        if(repo_size <= v_size){
            FILE_LOG(logDEBUG2) << "no need to add vertices: repository: " << repo_size << " graph " << v_size;
            return;
        }
        while(v_it!=vertices.cend()){
            FILE_LOG(logDEBUG2) << vertices.get_key(v_it);
            ++v_it;
        }
        FILE_LOG(logDEBUG2) << "start adding " << repo_size << " " << v_size << " vertices from rev: " << rev;
        for(unsigned int i = 0; i< repo_size - v_size;++i){
            vertex_descriptor d = boost::add_vertex(graph);
            map.insert(vertices.get_max_identifier(),d);
            FILE_LOG(logDEBUG2) << "ADD VARTEX " << d;
        }
        assert(repo_size==boost::num_vertices(graph));
    }

//    Graph& g;
    vertices_container vertices;
    edges_container edges;
    int _head_rev;
};

template<typename Graph, typename property_type>
struct helper{
    static void set_vertex_properties(Graph& graph,
                                      const typename graph_archive<Graph>::vertices_container& vertices,
                                      int rev){
#ifdef DEBUG
            FILE_LOG(logDEBUG3) << "started setting vertex properties for revision " << rev;
#endif
        typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
        std::pair<vertex_iterator,vertex_iterator> vi = boost::vertices(graph);
        for(vertex_iterator v_it = vi.first; v_it != vi.second; ++v_it){
            typename graph_archive<Graph>::vertices_container::key_type key(*v_it,rev);
#ifdef DEBUG
            FILE_LOG(logDEBUG3) << "setting vertex property " << *v_it << " rev: " << rev;
#endif
            typename graph_archive<Graph>::vertices_container::const_iterator it = vertices.lower_bound(key);
            assert(it!=vertices.cend());
#ifdef DEBUG
            FILE_LOG(logDEBUG3) << "found vertex key " << it->first << " and property: " << it->second;
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
};

template<typename Graph>
struct helper<Graph,boost::no_property>{
    static void set_vertex_properties(Graph&,const typename graph_archive<Graph>::vertices_container&, int){
    }
    template<typename descriptor>
    static boost::no_property get_properties(const Graph& , descriptor ){
        return boost::no_property();
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


#endif // ARCHIVE_H
