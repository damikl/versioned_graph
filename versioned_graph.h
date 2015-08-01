#ifndef VERSIONED_GRAPH_H
#define VERSIONED_GRAPH_H
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include "utils.h"
#include <boost/iterator/filter_iterator.hpp>

namespace boost {

template<class T>
struct property_records{
    typedef std::list<std::pair<revision,T> > type;
};

template<>
struct property_records<boost::no_property>{
    typedef std::list<revision> type;
};

template<class T>
struct property_optional_records{
    typedef std::list<std::pair<revision,T> > type;
};

template<>
struct property_optional_records<boost::no_property>{
    typedef boost::no_property type;
};

template<typename property_type>
revision get_revision(const std::pair<revision,property_type>& value){
    FILE_LOG(logDEBUG4) << "get_revision: " << value.first;
    return value.first;
}

template<typename graph_type,typename value_type>
struct filter_predicate{
    revision rev;
    const graph_type* g;
//    typedef typename value_type entry;
public:
    filter_predicate() : rev(std::numeric_limits<int>::max()),g(nullptr) {}
    filter_predicate(const filter_predicate &p) : rev(p.rev),g(p.g){}
    filter_predicate(const graph_type* graph,const revision& r = std::numeric_limits<int>::max()) : rev(r),g(graph) {}
    bool operator()(const value_type& v) {
        if(rev.get_rev()==std::numeric_limits<int>::max() && g==nullptr){
            FILE_LOG(logDEBUG4) << "default filter for edge, match all";
            return true;
        }
        assert(rev>0);
        auto list = g->get_history(v);
        for(auto iter = list.begin(); iter!=list.end();++iter){
            revision r = get_revision(*iter);
            if(r<=rev){
                if(is_deleted(r)){
                    FILE_LOG(logDEBUG4) << "filter_predicate: found deleted rev: " << r << " and while wanted: " << rev;
                    return false;
                }
                FILE_LOG(logDEBUG4) << "filter_predicate: found rev: " << r << " and while wanted: " << rev;
                return true;
            } else {
                FILE_LOG(logDEBUG4) << "filter_predicate: skipped rev: " << r << " and while wanted: " << rev;
            }
        }
        FILE_LOG(logDEBUG4) << "filter_predicate: not found " << rev;
        return false;
    }
};
template<typename graph_type>
struct filter_predicate<graph_type,typename boost::graph_traits<graph_type>::vertex_descriptor>{
    revision rev;
    typedef typename boost::graph_traits<graph_type>::vertex_descriptor value_type;
    const graph_type* g;
//    typedef typename value_type entry;
public:
    filter_predicate() : rev(std::numeric_limits<int>::max()),g(nullptr) {}
    filter_predicate(const filter_predicate &p) : rev(p.rev),g(p.g){}
    filter_predicate(const graph_type* graph,const revision& r = std::numeric_limits<int>::max()) : rev(r),g(graph) {}
    bool operator()(const value_type& v) {
        if(rev.get_rev()==std::numeric_limits<int>::max() && g==nullptr){
            FILE_LOG(logDEBUG4) << "default filter for vertex: " << v << ", match all";
            return true;
        }
        assert(rev>0);
        auto list = g->get_history(v);
        for(auto iter = list.begin(); iter!=list.end();++iter){
            revision r = get_revision(*iter);
            if(r<=rev){
                if(is_deleted(r)){
                    FILE_LOG(logDEBUG4) << "filter_predicate: found deleted rev: " << r << " and while wanted: " << rev;
                    return false;
                }
                FILE_LOG(logDEBUG4) << "filter_predicate: found rev: " << r << " and while wanted: " << rev;
                return true;
            } else {
                FILE_LOG(logDEBUG4) << "filter_predicate: skipped rev: " << r << " and while wanted: " << rev;
            }
        }
        FILE_LOG(logDEBUG4) << "filter_predicate: not found " << rev;
        return false;
    }
};


/*
template<>
struct filter_predicate<std::list<revision> >{
    revision rev;
//    typedef typename value_type entry;
public:
    filter_predicate(): rev(std::numeric_limits<int>::max()){}
    filter_predicate(const filter_predicate &p) : rev(p.rev){}
    filter_predicate(const revision& r) : rev(r) {}
    bool operator()(const std::list<revision>& list) {
        for(auto iter = list.begin(); iter!=list.end();++iter){
            revision r = *iter;
            if(r<=rev){
                FILE_LOG(logDEBUG4) << "filter_predicate: found rev: " << r << " and while wanted: " << rev;
                return true;
            } else {
                FILE_LOG(logDEBUG4) << "filter_predicate: skipped rev: " << r << " and while wanted: " << rev;
            }
        }
        FILE_LOG(logDEBUG4) << "filter_predicate: not found " << rev;
        return false;
    }
};
*/

template<typename OutEdgeList = boost::vecS,
         typename VertexList = boost::vecS,
         typename Directed = boost::directedS,
         typename VertexProperties = boost::no_property,
         typename EdgeProperties = boost::no_property,
         typename GraphProperties = boost::no_property,
         typename EdgeList = boost::listS>
class versioned_graph  : public boost::adjacency_list<OutEdgeList,VertexList,Directed,
        typename property_records<VertexProperties>::type,
        typename property_records<EdgeProperties>::type,
        typename property_optional_records<GraphProperties>::type,
    EdgeList>{
    template<typename bundled_type,typename descriptor>
    class graph_reference{
        graph_reference(versioned_graph& graph,const descriptor& desc) : g(graph),desc(desc) {}
    public:
        ~graph_reference(){}
        graph_reference& operator=( const bundled_type& x ){
            g.set_latest(desc,x);
            return *this;
        }
        graph_reference& operator=( const graph_reference& x ){
            g.set_latest(desc,g.get_latest_revision(desc),x);
            return *this;
        }
        operator bundled_type() const{
            return g.get_latest_bundled_value(desc);
        }
        versioned_graph& g;
        descriptor desc;
        friend class versioned_graph;
    };

public:
    typedef typename boost::adjacency_list<OutEdgeList,VertexList,Directed,
                                    typename property_records<VertexProperties>::type,
                                    typename property_records<EdgeProperties>::type,
                                    typename property_optional_records<GraphProperties>::type,
                                EdgeList> graph_type;

    typedef typename property_records<VertexProperties>::type vertices_history_type;
    typedef typename property_records<EdgeProperties>::type edges_history_type;
    typedef versioned_graph self;

    typedef VertexProperties vertex_bundled;
    typedef EdgeProperties edge_bundled;

    typedef typename boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<graph_type>::edge_descriptor edge_descriptor;
    typedef typename graph_type::degree_size_type degree_size_type;
    typedef typename graph_type::directed_category directed_category;
    typedef typename graph_type::edge_parallel_category edge_parallel_category;
    typedef typename graph_type::traversal_category traversal_category;
    typedef typename graph_type::vertices_size_type vertices_size_type;
    typedef typename graph_type::edges_size_type edges_size_type;
    typedef graph_reference<vertex_bundled,vertex_descriptor> vertex_bundled_reference;
    typedef graph_reference<edge_bundled,edge_descriptor> edge_bundled_reference;

    typedef typename boost::filter_iterator<
                                    filter_predicate<self,vertex_descriptor>,
                                                     typename boost::graph_traits<graph_type>::vertex_iterator>
                                                                                        vertex_iterator;
    typedef typename boost::filter_iterator<
                                    filter_predicate<self,edge_descriptor>,
                                                     typename boost::graph_traits<graph_type>::edge_iterator>
                                                                                        edge_iterator;

    typedef typename boost::filter_iterator<
                                    filter_predicate<self,edge_descriptor>,
                                                     typename boost::graph_traits<graph_type>::out_edge_iterator>
                                                                                        out_edge_iterator;

    typedef typename boost::filter_iterator<
                                    filter_predicate<self,edge_descriptor>,
                                                     typename boost::graph_traits<graph_type>::adjacency_iterator>
                                                                                        adjacency_iterator;



    auto vertices_begin() const {
        typename graph_type::vertex_iterator iter = boost::vertices(*dynamic_cast<const graph_type*>(this)).first;
        return iter;
        //return typename graph_type::vertex_iterator(this->vertex_set().begin());
    }
    auto vertices_end() const {
        typename graph_type::vertex_iterator iter = boost::vertices(*dynamic_cast<const graph_type*>(this)).second;
        return iter;
        //    return typename graph_type::vertex_iterator(this->vertex_set().end());
    }

    auto edges_begin() const {
        typename graph_type::edge_iterator iter = boost::edges(*dynamic_cast<const graph_type*>(this)).first;
        return iter;
        //    return typename graph_type::edge_iterator(this->m_edges.begin());
    }
    auto edges_end() const {
        typename graph_type::edge_iterator iter = boost::edges(*dynamic_cast<const graph_type*>(this)).second;
        return iter;
        //    return typename graph_type::edge_iterator(this->m_edges.end());
    }

    versioned_graph() : /*g(),*/current_rev(1),v_num(0),e_num(0) {}
    vertex_descriptor generate_vertex(VertexProperties prop){
        vertex_descriptor v = boost::add_vertex(*this);
        vertices_history_type& list = get_history(v);
        assert(list.empty());
        auto p = std::make_pair(current_rev,prop);
        list.push_front(p);
        ++v_num;
        return v;
    }
    vertex_descriptor generate_vertex(){
        vertex_descriptor v = boost::add_vertex(*this);
        vertices_history_type& list = (*this)[v];
        assert(list.empty());
        list.push_front(current_rev);
        ++v_num;
        return v;
    }

    std::pair<edge_descriptor,bool>
    generate_edge(VertexProperties prop,vertex_descriptor u, vertex_descriptor v){
        auto p = boost::add_edge(u,v,*this);
        if(p.second){
            edges_history_type& list = get_history(p.first);
            assert(list.empty());
            list.push_front(std::make_pair(current_rev,prop));
            FILE_LOG(logDEBUG4) << "add edge with rev " << current_rev;
            ++e_num;
        } else {
            FILE_LOG(logDEBUG4) << "edge already exist";
        }
        return p;
    }

    std::pair<edge_descriptor,bool>
    generate_edge(vertex_descriptor u, vertex_descriptor v){
        auto& p = boost::add_edge(u,v,static_cast<graph_type>(*this));
        if(p.second){
            edges_history_type list = (*this)[p.first];
            assert(list.empty());
            list.push_front(current_rev);
            FILE_LOG(logDEBUG4) << "add edge with rev " << current_rev;
            ++e_num;
        } else {
            FILE_LOG(logDEBUG4) << "edge already exist";
        }
        return p;
    }
private:
    template<typename descriptor,typename bundled_type>
    void set_deleted(descriptor& e,bundled_type dummy_value){
        auto& list = get_history(e);
        FILE_LOG(logDEBUG4) << "set deleted: " << list.size() << " records in history";
        assert(!is_deleted(get_revision(list.front())));
        revision r = make_deleted(current_rev);
        FILE_LOG(logDEBUG4) << "negated to " << r;
        auto p = std::make_pair(r,dummy_value);
        FILE_LOG(logDEBUG4) << "entry created";
        list.push_front(p);
    }
    template<typename descriptor,typename bundled_type>
    void set_latest(descriptor& e,bundled_type value){
        auto& list = get_history(e);
        if(this->get_latest_revision(e)< current_rev){
            auto p = std::make_pair(current_rev,value);
            list.push_front(p);
        } else {
            list.front().second = value;
        }
    }
    template<typename descriptor>
    auto get_latest_bundled_value(descriptor d){
        const auto& list = get_history(d);
        assert(get_revision(list.front())<=current_rev);
        return list.front().second;
    }

public:
    void set_deleted(edge_descriptor e){
        if(get_history(e).size()>1 || get_latest_revision(e) < current_rev){
            set_deleted(e,edge_bundled());
            assert(check_if_currently_deleted(e));
        } else {
            boost::remove_edge(e,*dynamic_cast<graph_type*>(this));
        }
        --e_num;
        FILE_LOG(logDEBUG4) << "set deleted: finnished";
    }
    template<typename descriptor>
    bool check_if_currently_deleted(descriptor d) const {
        return is_deleted(get_revision(get_history(d).front()));
    }
    void set_deleted(vertex_descriptor v){
        if(get_history(v).size()>1 || get_latest_revision(v) < current_rev){
            set_deleted(v,vertex_bundled());
            assert(check_if_currently_deleted(v));
        } else {
            boost::remove_vertex(v,*dynamic_cast<graph_type*>(this));
        }
        --v_num;
        FILE_LOG(logDEBUG4) << "set deleted: finnished";
    }
    vertices_size_type num_vertices() const {
        FILE_LOG(logDEBUG4) << "get num vertices " << v_num;
        return v_num;
    }
    edges_size_type num_edges() const {
        FILE_LOG(logDEBUG4) << "get num edges " << e_num;
        return e_num;
    }
    void commit(){
        ++current_rev;
    }
private:
    void clean_history( edges_history_type& hist, edge_descriptor desc){
        revision r = get_revision(hist.front());
        if (is_deleted(r)) {
           ++e_num;
        }
        FILE_LOG(logDEBUG4) << "remove edges history entry: ( "
                            << boost::source(desc,*this) << ", "
                            << boost::target(desc,*this) << ") for rev: " << r;
        hist.pop_front();
        r = get_revision(hist.front());
        FILE_LOG(logDEBUG4) << "after edge history removal: ( "
                            << boost::source(desc,*this) << ", "
                            << boost::target(desc,*this) << ") for rev: " << r;
    }
    void clean_history( vertices_history_type& hist, vertex_descriptor desc){
        revision r = get_revision(hist.front());
        if (is_deleted(r)) {
           ++v_num;
        }
        FILE_LOG(logDEBUG4) << "remove vertices history entry: ("<< desc << ") for rev: " << r;
        hist.pop_front();
        r = get_revision(hist.front());
        FILE_LOG(logDEBUG4) << "after vertices history removal: ("<< desc << ") for rev: " << r;
    }
public:
    void undo_commit(){
        auto ei = boost::edges(*this);
        for(auto edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            edges_history_type& hist = get_history(*edge_iter);
            while(get_latest_revision(*edge_iter)>=current_rev){
                clean_history(hist,*edge_iter);
            }
        }
        auto vi = boost::vertices(*this);
        for(auto vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
            vertices_history_type& hist = get_history(*vertex_iter);
            while(get_latest_revision(*vertex_iter)>=current_rev){
                clean_history(hist,*vertex_iter);
            }
        }
        --current_rev;
    }
    template<typename descriptor>
    revision get_latest_revision(const descriptor& v) const {
        const vertices_history_type& list = get_history(v);
        return get_revision(list.front());
    }

    vertices_history_type& get_history(const vertex_descriptor& idx){
        return (*dynamic_cast<graph_type*>(this))[idx];
    }
    const graph_type& get_self() const{
        return *dynamic_cast<const graph_type*>(this);
    }
    const vertices_history_type& get_history(const vertex_descriptor& idx)const {
        return (*dynamic_cast<const graph_type*>(this))[idx];
    }

    edges_history_type& get_history(const edge_descriptor& idx){
        return (*dynamic_cast<graph_type*>(this))[idx];
    }

    const edges_history_type& get_history(const edge_descriptor& idx)const {
        return (*dynamic_cast<const graph_type*>(this))[idx];
    }

    vertex_bundled_reference operator[](const vertex_descriptor& v) {
        graph_reference<vertex_bundled,vertex_descriptor> ref(*this,v);
        return ref;
    }
    const vertex_bundled& operator[](const vertex_descriptor& v) const {
        return get_latest_bundled_value(v);
    }

    edge_bundled_reference operator[](const edge_descriptor& e) {
        graph_reference<edge_bundled,edge_descriptor> ref(*this,e);
        return ref;
    }
    const edge_bundled& operator[](const edge_descriptor& e) const {
        return get_latest_bundled_value(e);
    }

private:
//    graph_type g;
    revision current_rev;
    vertices_size_type v_num;
    edges_size_type e_num;
};

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto add_vertex(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.generate_vertex();
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto add_vertex(VertexProperties p, versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.generate_vertex(p);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto num_vertices(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.num_vertices();
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
void remove_vertex(vertex_descriptor v, versioned_graph<OutEdgeList,VertexList,Directed,
                 VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    g.set_deleted(v);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
void clear_out_edges(vertex_descriptor u, versioned_graph<OutEdgeList,VertexList,Directed,
                     VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
                        VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph;
    auto ei = out_edges(u,g);
    for(typename graph::edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        remove_edge(*edge_iter,g);
    }
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
void clear_in_edges(vertex_descriptor u, versioned_graph<OutEdgeList,VertexList,Directed,
                    VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
                        VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph;
    auto ei = in_edges(u,g);
    for(typename graph::edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        remove_edge(*edge_iter,g);
    }
}
template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
void clear_vertex(vertex_descriptor u, versioned_graph<OutEdgeList,VertexList,Directed,
                  VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    clear_in_edges(u,g);
    clear_out_edges(u,g);
}


template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto num_edges(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.num_edges();
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto vertices(const versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
            VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;

    filter_predicate<graph_type,typename graph_type::vertex_descriptor> predicate(&g);
    typename graph_type::vertex_iterator iter_begin(predicate, g.vertices_begin(), g.vertices_end());
    typename graph_type::vertex_iterator iter_end(predicate, g.vertices_end(), g.vertices_end());
    return std::make_pair(iter_begin,iter_end);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto edges(const versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
            VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;

    filter_predicate<graph_type,typename graph_type::edge_descriptor> predicate(&g);
    typename graph_type::edge_iterator iter_begin(predicate, g.edges_begin(), g.edges_end());
    typename graph_type::edge_iterator iter_end(predicate, g.edges_end(), g.edges_end());
    return std::make_pair(iter_begin,iter_end);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
auto adjacent_vertices(vertex_descriptor u,
                  const versioned_graph<OutEdgeList,VertexList,Directed,
                  VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
            VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;

    auto adj = boost::adjacent_vertices(u,g.get_self());
    filter_predicate<graph_type,typename graph_type::edge_descriptor> predicate(&g);
    typename graph_type::adjacency_iterator iter_begin(predicate, adj.first, adj.second);
    typename graph_type::adjacency_iterator iter_end(predicate, adj.second, adj.second);
    return std::make_pair(iter_begin,iter_end);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
auto add_edge(vertex_descriptor u,vertex_descriptor v, versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.generate_edge(u,v);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto commit(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.commit();
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto undo_commit(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.undo_commit();
}


template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
auto add_edge(EdgeProperties p,vertex_descriptor u,vertex_descriptor v, versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.generate_edge(p,u,v);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
void remove_edge(vertex_descriptor u,vertex_descriptor v, versioned_graph<OutEdgeList,VertexList,Directed,
                 VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    auto p = edge(u,v,g);
    if(p.second){
        FILE_LOG(logDEBUG4) << "remove_edge: found existing";
        auto edge_desc = p.first;
        FILE_LOG(logDEBUG4) << "remove_edge: got desc: (" << boost::source(edge_desc,g) << ", " << boost::target(edge_desc,g) << ")";
        g.set_deleted(edge_desc);
    }
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
auto edge(vertex_descriptor u,vertex_descriptor v, const versioned_graph<OutEdgeList,VertexList,Directed,
                 VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g) {

    typedef versioned_graph<OutEdgeList,VertexList,Directed,
            VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;
    auto p = boost::edge(u,v,dynamic_cast<const typename graph_type::graph_type&>(g));
    if(p.second){
        FILE_LOG(logDEBUG4) << "edge (" << u << ", " << v  <<  "): found existing intenally";
        auto edge_desc = p.first;
        const typename graph_type::edges_history_type& list = g.get_history(edge_desc);
        FILE_LOG(logDEBUG4) << /*list.size()  <<*/ "records in history of edge";
        if(is_deleted(get_revision(list.front()))){
            FILE_LOG(logDEBUG4) << "edge: is deleted";
            return std::make_pair(typename graph_type::edge_descriptor(),false);
        }
        FILE_LOG(logDEBUG4) << "edge: not deleted";
    } else {
        FILE_LOG(logDEBUG4) << "edge: not found";
    }
    return p;
}

}

#endif // VERSIONED_GRAPH_H
