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

template<typename Graph>
class graph_archive
{

    typedef typename boost::graph_traits<Graph>::edge_iterator edge_iterator;
    typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
    typedef vertex_id<Graph> vertex_key;
    typedef edge_id<Graph> edge_key;
public:
    typedef typename boost::vertex_bundle_type<Graph>::type vertex_properties;
    typedef typename boost::edge_bundle_type<Graph>::type edge_properties;
    typedef history_holder<vertex_key,vertex_properties> vertices_container;
    typedef history_holder<edge_key,edge_properties> edges_container;

    friend struct helper<Graph,vertex_properties>;
    graph_archive(Graph& graph) : g(graph), _head_rev(0){
    }
    void commit(){
        std::pair<edge_iterator, edge_iterator> ei = boost::edges(g);
        std::pair<vertex_iterator, vertex_iterator> vi = boost::vertices(g);
#ifdef DEBUG
        std::cout << "before COMMIT" << std::endl;
        print_edges();
#endif
        ++_head_rev;
        typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
        {
            std::set<vertex_descriptor> ah_vertices; // vertices that are already here
            for(vertex_iterator vertex_iter = vi.first; vertex_iter != vi.second;++vertex_iter){
                typename vertices_container::const_iterator v_it = lower_bound(*vertex_iter);
                if(v_it == vertices.end() || vertices.changed(*vertex_iter,v_it,g)){
                    commit(*vertex_iter,head_rev());// new vertex
                   }else{
                    ah_vertices.insert(*vertex_iter);// note already existing vertex
                }
            }
            // mark currently missing vertices as removed (negative revision)
            typename vertices_container::const_iterator v_it = vertices.begin();
            while(v_it !=vertices.end()){
               vertex_key curr = vertices.get_key(v_it);
               if(abs(curr.revision) < head_rev() && ah_vertices.find(curr.id)== ah_vertices.end()) {
                    del_commit(curr.id,-head_rev());
               }
               v_it = vertices.upper_bound(vertex_key::get_min(curr.id));
            }
        }
        {
            typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
            std::set<std::pair<vertex_descriptor,vertex_descriptor> > ah_edges; // edges that are already here
            for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
                typename edges_container::const_iterator e_it = lower_bound(*edge_iter);
                if(e_it == edges.end() || edges.changed(*edge_iter,e_it,g)){
                    commit(*edge_iter,head_rev());
                } else {
                    ah_edges.insert(std::make_pair(source(*edge_iter,g),target(*edge_iter,g)));
                }
            }
            // mark currently missing edges as removed (negative revision)
            typename edges_container::const_iterator e_it = edges.begin();
            while(e_it !=edges.end()){
                edge_key curr = edges.get_key(e_it);
                std::pair<vertex_descriptor,vertex_descriptor> edge = std::make_pair(curr.source,curr.target);
                if(abs(curr.revision) < head_rev() && ah_edges.find(edge)== ah_edges.end()) {
                    del_commit(edge,-head_rev());
                }
                e_it = edges.upper_bound(edge_key::get_min(curr.source,curr.target));
            }
        }
#ifdef DEBUG
        std::cout << "after COMMIT" << std::endl;
        print_edges();
#endif
    }
    /**
      check if edge exist in head revision
    **/
    bool contains_edge(const typename boost::graph_traits<Graph>::edge_descriptor e) const{
        edge_key ed = edge_key::get_max(source(e, g),target(e, g));
        edge_key tmp = edges.lower_bound(ed)->first;
        return thesame(ed,tmp) && tmp.revision >=0;
    }
    /**
      check if vertex exist in head revision
    **/
    bool contains_vertex(const typename boost::graph_traits<Graph>::vertex_descriptor v)const{
        vertex_key ed = vertex_key::get_max(v);
        vertex_key tmp = vertices.lower_bound(ed)->first;
        return thesame(ed,tmp) && tmp.revision >=0;
    }
    bool contains_edge(const edge_key& e) const{
        typename edges_container::const_iterator e_it = edges.lower_bound(e);
        if(e_it == edges.end())
            return false;
        return e_it->first.revision >=0;
    }
    void print_edges(){
        typename edges_container::iterator e_it = edges.begin();
        while(e_it != edges.end()){
            edge_key curr_edge = edges.get_key(e_it);
            std::cout << "edge: " << curr_edge.source << " " << curr_edge.target << " "<< curr_edge.revision << std::endl;
            ++e_it;
        }
    }
    Graph checkout(){
        return checkout(head_rev());
    }
    Graph checkout(int rev){
        Graph n;
        typename edges_container::iterator e_it = edges.begin(); // init with head revision
        while(e_it != edges.end()){
            edge_key curr_edge = edges.get_key(e_it);
            if(abs(curr_edge.revision) > abs(rev)) { // if looking for older revisions
                edge_key key(curr_edge.source,curr_edge.target,rev);
                e_it = edges.lower_bound(key);
                if(e_it == edges.end())
                    break;
                edge_key tmp = edges.get_key(e_it);
                if(thesame(key,tmp))// unless on next edge
                    curr_edge = tmp;
                else
                    continue;// when analysed edge was created in more recent revision
            }
            if(curr_edge.revision >=0){// negative revision == removed
                boost::add_edge(curr_edge.source,curr_edge.target,n);
#ifdef DEBUG
                std::cout << "checked out: " << curr_edge.source << " " << curr_edge.target << " rev: "<< curr_edge.revision << " wanted: " << rev << std::endl;
#endif
                edges.set_edge_property(e_it,n,curr_edge.source,curr_edge.target);
            }else{
#ifdef DEBUG
                std::cout << "NOT checked out: " << curr_edge.source << " " << curr_edge.target << " rev: "<< curr_edge.revision << " wanted: " << rev << std::endl;
#endif
            }
            //jump to head revision of next edge
            e_it = edges.upper_bound(edge_key::get_min(curr_edge.source,curr_edge.target));
        }
        helper<Graph,vertex_properties>::set_vertex_properties(n,vertices, rev);
        return n;
    }
    void replace_with(int revision){
        typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
        {
            typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
            std::set<std::pair<vertex_descriptor,vertex_descriptor> > ah_edges; // edges that are already here
            edge_remover<graph_archive<Graph> > rm(revision,edges,*this);
            remove_edge_if(rm,g);
            std::pair<edge_iterator, edge_iterator> ei = boost::edges(g);
            for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
                typename edges_container::const_iterator e_it = lower_bound(*edge_iter,revision);
                if(edges.changed(*edge_iter,e_it,g)){
                    edges.set_edge_property(e_it,g,source(*edge_iter,g),target(*edge_iter,g));
                }
                ah_edges.insert(std::make_pair(source(*edge_iter,g),target(*edge_iter,g)));
            }
            // mark currently missing edges as removed (negative revision)
            typename edges_container::const_iterator e_it = edges.begin();
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
                typename vertices_container::const_iterator v_it = lower_bound(*vertex_iter,revision);
                if(v_it == vertices.end()){
                    remove_vertex(*vertex_iter,g);
                }
            }
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

    typename edges_container::const_iterator
    lower_bound(const typename boost::graph_traits<Graph>::edge_descriptor e) const {
        return lower_bound(e,head_rev());
    }

    typename edges_container::const_iterator
    lower_bound(const typename boost::graph_traits<Graph>::edge_descriptor e, int revision) const {
        edge_key ed = edge_key(source(e, g),target(e, g),revision);
        typename edges_container::const_iterator it = edges.lower_bound(ed);
        if(it == edges.end())
            return it;
        edge_key curr = edges.get_key(it);
        if(source(e, g) != curr.source ||
           target(e, g) != curr.target ||
           curr.revision < 0)
            return edges.end();
        return it;
    }

    typename vertices_container::const_iterator
    lower_bound(const typename boost::graph_traits<Graph>::vertex_descriptor v) const{
        return lower_bound(v,head_rev());
    }
    typename vertices_container::const_iterator
    lower_bound(const typename boost::graph_traits<Graph>::vertex_descriptor v, int revision) const{
        vertex_key max_key = vertex_key(v,revision);
        typename vertices_container::const_iterator it = vertices.lower_bound(max_key);
        if(it == vertices.end())
            return it;
        vertex_key curr = vertices.get_key(it);
        if(v != curr.id || curr.revision < 0)
            return vertices.end();
        return it;
    }

    vertex_key commit(typename boost::graph_traits<Graph>::vertex_descriptor v, int revision){
        vertex_key el(v,revision);
        vertices.insert(el, helper<Graph,vertex_properties>::get_properties(g,v));
        return el;
    }
    vertex_key del_commit(typename boost::graph_traits<Graph>::vertex_descriptor v, int revision){
        vertex_key el(v,revision);
        vertices.insert(el, vertex_properties());
        return el;
    }
    edge_key commit(typename boost::graph_traits<Graph>::edge_descriptor v, int revision){
        edge_key el(source(v, g),target(v, g),revision);
        edges.insert(el, helper<Graph,edge_properties>::get_properties(g,v));
        return el;
    }
    edge_key del_commit(std::pair<  typename boost::graph_traits<Graph>::vertex_descriptor,
                                typename boost::graph_traits<Graph>::vertex_descriptor> v,
                    int revision){
        edge_key el(v.first,v.second,revision);
        edges.insert(el, edge_properties());
        return el;
    }

    Graph& g;
    vertices_container vertices;
    edges_container edges;
    int _head_rev;
};

template<typename Graph, typename property_type>
struct helper{
    static void set_vertex_properties(Graph& graph,
                                      const typename graph_archive<Graph>::vertices_container& vertices,
                                      int rev){
        typename graph_archive<Graph>::vertices_container::const_iterator v_it = vertices.begin();
        while(v_it != vertices.end()){
            typename graph_archive<Graph>::vertex_key curr_vertex = v_it->first;
            if(abs(curr_vertex.revision) > abs(rev)){
                 v_it = vertices.lower_bound(vertex_id<Graph>(curr_vertex.id,rev));
                 if(v_it != vertices.end())
                     curr_vertex = v_it->first;
                 else
                     break;
             }
#ifdef DEBUG
            std::cout << "setting vertex property" << curr_vertex.id << " rev: " << curr_vertex.revision << std::endl;
#endif
            if(curr_vertex.revision >=0){
                graph[curr_vertex.id] = v_it->second;
            }
            v_it = vertices.upper_bound(graph_archive<Graph>::vertex_key::get_min(curr_vertex.id));
        }
    }
    template<typename descriptor>
    static property_type get_properties(const Graph& graph,descriptor v){
        return graph[v];
    }
};
/*
template<typename Graph,class PropertyTag, class T, class NextProperty>
struct helper<Graph,boost::property<PropertyTag,T,NextProperty> >{
    typedef typename boost::property<PropertyTag,T,NextProperty> property;
    static void set_vertex_properties(Graph&,const typename graph_archive<Graph>::vertices_container&, int){

    }
    template<typename descriptor>
    static property get_properties(const Graph& graph, descriptor v){
        return graph[v];
    }
};
*/
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
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
    typedef typename boost::edge_bundle_type<Graph>::type edge_properties;
    std::pair<edge_iterator, edge_iterator> e_g1 = boost::edges(g1);
    if(boost::num_edges(g1)!=boost::num_edges(g2) || boost::num_vertices(g1)!=boost::num_vertices(g2))
        return false;
    for(edge_iterator it = e_g1.first; it!=e_g1.second;++it){
        std::pair<edge_descriptor,bool> p = boost::edge(source(*it,g1),target(*it,g1),g2);
        if(!p.second || !compare(g2[p.first],g1[*it]))
            return false;
    }
    std::pair<vertex_iterator, vertex_iterator> v_g1 = boost::vertices(g1);
    for(vertex_iterator it = v_g1.first; it!=v_g1.second;++it){
        if(!compare(g2[*it],g1[*it]))
            return false;
    }
    return true;
}


#endif // ARCHIVE_H
