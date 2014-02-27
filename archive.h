#ifndef ARCHIVE_H
#define ARCHIVE_H
#include "iostream"
#include <limits>
#include <vector>
#include <algorithm>
#include "boost/graph/graph_traits.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/filtered_graph.hpp"

template<typename Graph>
struct archive_vertex{
    typedef typename Graph::vertex_descriptor vertex_type;
    vertex_type id;
    int revision;
    archive_vertex(vertex_type id, int rev) : id(id),revision(rev){}
    archive_vertex(){}
    bool operator<(const archive_vertex& obj)const{
        if(this->id == obj.id)
            return abs(this->revision) > abs(obj.revision);
        return this->id < obj.id;
    }
    bool operator==(const archive_vertex& obj)const{
        if(this->id == obj.id && this->revision == obj.revision)
            return true;
        return false;
    }
    static archive_vertex get_max(vertex_type id){
         archive_vertex arch;
         arch.id = id;
         arch.revision = std::numeric_limits<int>::max();
         return arch;
    }
    static archive_vertex get_min(vertex_type id){
         archive_vertex arch;
         arch.id = id;
         arch.revision = 0;
         return arch;
    }
};
template<typename Graph>
struct archive_edge{
   typedef typename Graph::vertex_descriptor vertex_type;
   vertex_type source;
   vertex_type target;
   int revision;
   archive_edge(){}
   archive_edge(vertex_type source,
                vertex_type target,
                int revision):  source(source),
                                target(target),
                                revision(revision){}

   bool operator<(const archive_edge& obj) const {
       if(this->source == obj.source){
            if(this->target == obj.target)
                return abs(this->revision) > abs(obj.revision);
            return this->target < obj.target;
       }
       return this->source < obj.source;
   }
   bool operator==(const archive_edge& obj)const{
       if(this->source == obj.source &&
          this->target == obj.target &&
          this->revision == obj.revision)
            return true;
       return false;
   }
   archive_edge& operator=(archive_edge rhs)
   {
     this->source = rhs.source;
     this->target = rhs.target;
     this->revision = rhs.revision;
     return *this;
   }
   static archive_edge get_max(vertex_type source, vertex_type target){
        archive_edge arch;
        arch.source = source;
        arch.target = target;
        arch.revision = std::numeric_limits<int>::max();
        return arch;
   }
   static archive_edge get_min(vertex_type source, vertex_type target){
        archive_edge arch;
        arch.source = source;
        arch.target = target;
        arch.revision = 0;
        return arch;
   }
};
template<typename Graph>
bool thesame(const archive_vertex<Graph>& f,const archive_vertex<Graph>& s){
    return f.id == s.id;
}
template<typename Graph>
bool thesame(const archive_edge<Graph>& f,const archive_edge<Graph>& s){
    return f.source == s.source && f.target == s.target;
}

template<typename Graph, typename vertex_prop_type>
struct helper;

template<typename Graph, typename descriptor,typename property>
struct value_checker{
    static bool changed(descriptor v, property p,const Graph& g) {
        return g[v] != p;
    }
};
template<typename Graph, typename descriptor>
struct value_checker<Graph,descriptor,boost::no_property>{
    static bool changed(descriptor, boost::no_property,const Graph&) {
        return false;
    }
};

template<typename Graph>
class graph_archive
{

    typedef typename boost::graph_traits<Graph>::edge_iterator edge_iterator;
    typedef archive_vertex<Graph> vertex_key;
    typedef archive_edge<Graph> edge_key;
public:
    typedef typename boost::vertex_bundle_type<Graph>::type vertex_properties;
    typedef typename boost::edge_bundle_type<Graph>::type edge_properties;
private:
    typedef typename std::map<vertex_key,vertex_properties> vertices_container;
    typedef typename std::map<edge_key,edge_properties> edges_container;

public:

    friend struct helper<Graph,vertex_properties>;
    graph_archive(const Graph& graph) : g(graph){
    }
    void commit(std::pair<edge_iterator, edge_iterator> ei){
        std::cout << "COMMIT" << std::endl;
        print_edges();
        int rev = head_rev()+1;
        typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
        typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
        std::set<vertex_descriptor> ah_vertices; // already here
        std::set<std::pair<vertex_descriptor,vertex_descriptor> > ah_edges;

        for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            vertex_descriptor v = source(*edge_iter, g);
            typename vertices_container::const_iterator v_it = lower_bound(v);
            if(v_it == vertices.end()){
                commit(v,rev);
            } else {
                if(g[v] != v_it->second)
                    commit(v,rev);
                else
                    ah_vertices.insert(v);
            }
            vertex_descriptor u = target(*edge_iter, g);
            typename vertices_container::const_iterator u_it = lower_bound(u);
            if(u_it == vertices.end()){
                commit(u,rev);
            } else {
                if(g[u] != u_it->second)
                    commit(u,rev);
                else
                    ah_vertices.insert(u);
            }
            edge_descriptor e = *edge_iter;
            typename edges_container::const_iterator e_it = lower_bound(e);
            if(e_it == edges.end()){
                commit(e,rev);
            } else {
                if(value_checker<Graph,edge_descriptor,edge_properties>::changed(e,e_it->second,g))
                    commit(e,rev);
                else
                    ah_edges.insert(std::make_pair(source(e,g),target(e,g)));
            }

        }

        typename edges_container::iterator e_it = edges.begin();
        while(e_it !=edges.end()){
            edge_key curr = e_it->first;
            std::pair<vertex_descriptor,vertex_descriptor> edge = std::make_pair(curr.source,curr.target);
            if(abs(curr.revision) < abs(rev) && ah_edges.find(edge)!= ah_edges.end())
            {
                del_commit(edge,-rev);
            }
            e_it = edges.upper_bound(edge_key::get_min(curr.source,curr.target));
        }
        typename vertices_container::iterator v_it = vertices.begin();
        while(v_it !=vertices.end()){
           vertex_key curr = v_it->first;
           if(abs(curr.revision) < abs(rev) && ah_vertices.find(curr.id)!= ah_vertices.end())
            {
                commit(curr.id,-rev);
            }
            v_it = vertices.upper_bound(vertex_key::get_min(curr.id));
        }
    }

    bool contains_edge(const typename boost::graph_traits<Graph>::edge_descriptor e) const{
        edge_key ed = edge_key::get_max(source(e, g),target(e, g));
        edge_key tmp = edges.lower_bound(ed)->first;
        return thesame(ed,tmp) && tmp.revision >=0;
    }
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
            edge_key curr_edge = e_it->first;
            std::cout << "int " << curr_edge.source << " " << curr_edge.target << " "<< curr_edge.revision << std::endl;
            ++e_it;
        }

    }
    Graph checkout(){
        return checkout(head_rev());
    }
    Graph checkout(int rev){
        Graph n;
        typename edges_container::iterator e_it = edges.begin();
        while(e_it != edges.end()){
           edge_key curr_edge = e_it->first;
           if(abs(curr_edge.revision) > abs(rev))
            {
               std::cout << "inner" << std::endl;
                e_it = edges.lower_bound(archive_edge<Graph>(curr_edge.source,curr_edge.target,rev));
                if(e_it != edges.end())
                    curr_edge = e_it->first;
                else
                    break;
            }
            boost::add_edge(curr_edge.source,curr_edge.target,n);
            std::cout << "internal" << curr_edge.source << " " << curr_edge.target << " "<< curr_edge.revision << "a" << std::endl;
            set_edge_property(e_it->second,n,curr_edge.source,curr_edge.target);
            //move to next edge
            e_it = edges.upper_bound(edge_key::get_min(curr_edge.source,curr_edge.target));
        }
        std::cout << "vertex" << std::endl;
        helper<Graph,vertex_properties>::set_vertex_properties(n,vertices, rev);
        std::cout << "vertex2" << std::endl;
        return n;
    }

    int head_rev() const{
        int v = 0, e = 0;
        if(!vertices.empty())
            v = vertices.begin()->first.revision;
        if(!edges.empty())
            e = edges.begin()->first.revision;
        std::cout << "revs: "<< v << " " << e << std::endl;
        return std::max(std::abs(v),std::abs(e));
    }

private:

    typename edges_container::const_iterator lower_bound(const typename boost::graph_traits<Graph>::edge_descriptor e) const {
        edge_key ed = edge_key::get_max(source(e, g),target(e, g));
        typename edges_container::const_iterator it = edges.lower_bound(ed);
        if(it == edges.end())
            return it;
        if(source(e, g) != it->first.source ||
           target(e, g) != it->first.target ||
           it->first.revision < 0)
            return edges.end();
        return it;
    }
    typename vertices_container::const_iterator lower_bound(const typename boost::graph_traits<Graph>::vertex_descriptor v) const{
        vertex_key ed = vertex_key::get_max(v);
        typename vertices_container::const_iterator it = vertices.lower_bound(ed);
        if(it == vertices.end())
            return it;
        if(v != it->first.id ||
           it->first.revision < 0)
            return vertices.end();
        return it;
    }

    vertex_key commit(typename boost::graph_traits<Graph>::vertex_descriptor v, int revision){
        vertex_key el;
        el.id = v;
        el.revision = revision;
        vertices.insert(std::make_pair(el, g[v]));
        return el;
    }
    edge_key commit(typename boost::graph_traits<Graph>::edge_descriptor v, int revision){
        edge_key el;
        el.source = source(v, g);
        el.target = target(v, g);
        el.revision = revision;
        edges.insert(std::make_pair(el, g[v]));
        return el;
    }
    edge_key del_commit(std::pair<  typename boost::graph_traits<Graph>::vertex_descriptor,
                                typename boost::graph_traits<Graph>::vertex_descriptor> v,
                    int revision){
        edge_key el;
        el.source = v.first;
        el.target = v.second;
        el.revision = revision;
        edges.insert(std::make_pair(el, edge_properties()));
        return el;
    }

    template<typename edge_prop_type>
    void set_edge_property(edge_prop_type property,
                           Graph& graph,
                           typename edge_key::vertex_type s,
                           typename edge_key::vertex_type t){
        boost::put(property,graph,s,t);
    }
    void set_edge_property(boost::no_property,
                           Graph&,
                           typename edge_key::vertex_type,
                           typename edge_key::vertex_type){
    }


    const Graph& g;
    vertices_container vertices;
    edges_container edges;

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
                 v_it = vertices.lower_bound(archive_vertex<Graph>(curr_vertex.id,rev));
                 if(v_it != vertices.end())
                     curr_vertex = v_it->first;
                 else
                     break;
             }
            std::cout << "set prop" << curr_vertex.id << " " << curr_vertex.revision << std::endl;
            graph[curr_vertex.id] = v_it->second;
            v_it = vertices.upper_bound(graph_archive<Graph>::vertex_key::get_min(curr_vertex.id));
        }
    }

};

template<typename Graph>
struct helper<Graph,boost::no_property>{
    static void set_vertex_properties(Graph&,const typename graph_archive<Graph>::vertices_container&){

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

template<typename Graph>
bool equal(const Graph& g1, const Graph& g2){
    typedef typename boost::graph_traits<Graph>::edge_iterator edge_iterator;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
    typedef typename boost::edge_bundle_type<Graph>::type edge_properties;
    std::pair<edge_iterator, edge_iterator> e_g1 = boost::edges(g1);
    if(boost::num_edges(g1)!=boost::num_edges(g2))
        return false;
    for(edge_iterator it = e_g1.first; it!=e_g1.second;++it){
        std::pair<edge_descriptor,bool> p = boost::edge(source(*it,g1),target(*it,g1),g2);
        if(!p.second || !compare(g2[p.first],g1[*it]))
            return false;
    }
    return true;
}


#endif // ARCHIVE_H
