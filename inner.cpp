//#include <boost/config.hpp>
#include <iostream>
#include <utility>
//#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include "utils.h"
#include <boost/iterator/filter_iterator.hpp>
#include <boost/graph/topological_sort.hpp>


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
        assert(rev>0);
        if(rev.get_rev()==std::numeric_limits<int>::max() && g==nullptr){
            FILE_LOG(logDEBUG4) << "default filter, match all";
            return true;
        }
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



    auto vertices_begin(){
        typename graph_type::vertex_iterator iter = boost::vertices(*dynamic_cast<graph_type*>(this)).first;
        return iter;
        //return typename graph_type::vertex_iterator(this->vertex_set().begin());
    }
    auto vertices_end(){
        typename graph_type::vertex_iterator iter = boost::vertices(*dynamic_cast<graph_type*>(this)).second;
        return iter;
        //    return typename graph_type::vertex_iterator(this->vertex_set().end());
    }

    auto edges_begin(){
        typename graph_type::edge_iterator iter = boost::edges(*dynamic_cast<graph_type*>(this)).first;
        return iter;
        //    return typename graph_type::edge_iterator(this->m_edges.begin());
    }
    auto edges_end(){
        typename graph_type::edge_iterator iter = boost::edges(*dynamic_cast<graph_type*>(this)).second;
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
    void undo(){
        auto ei = boost::edges(*this);
        for(auto edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            edges_history_type& hist = get_history(*edge_iter);
            while(get_latest_revision(*edge_iter)>=current_rev){
                revision r = get_revision(hist.front());
                if (is_deleted(r)) {
                   ++e_num;
                }
                FILE_LOG(logDEBUG4) << "remove edges history entry: ( "
                                    << boost::source(*edge_iter,*this) << ", "
                                    << boost::target(*edge_iter,*this) << ") for rev: " << r;
                hist.pop_front();
                r = get_revision(hist.front());
                FILE_LOG(logDEBUG4) << "after edge history removal: ( "
                                    << boost::source(*edge_iter,*this) << ", "
                                    << boost::target(*edge_iter,*this) << ") for rev: " << r;

            }
        }
        auto vi = boost::vertices(*this);
        for(auto vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
            vertices_history_type& hist = get_history(*vertex_iter);
            while(get_latest_revision(*vertex_iter)>=current_rev){
                revision r = get_revision(hist.front());
                if (is_deleted(r)) {
                   ++v_num;
                }
                FILE_LOG(logDEBUG4) << "remove vertices history entry: ("<< *vertex_iter << ") for rev: " << r;
                hist.pop_front();
                r = get_revision(hist.front());
                FILE_LOG(logDEBUG4) << "after vertices history removal: ("<< *vertex_iter << ") for rev: " << r;
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
auto vertices(versioned_graph<OutEdgeList,VertexList,Directed,
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
auto edges(versioned_graph<OutEdgeList,VertexList,Directed,
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
auto undo(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.undo();
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




void test_graph(){
    using namespace boost;
    using namespace std;
    typedef versioned_graph<boost::vecS, boost::vecS, boost::directedS,int,int> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
//    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
//    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    auto v1 = add_vertex(1,sg);
    auto v2 = add_vertex(2,sg);
    auto v3 = add_vertex(3,sg);
    auto v4 = add_vertex(4,sg);
    add_edge(9,v1,v2,sg);
    add_edge(8,v1,v3,sg);
    add_edge(7,v2,v4,sg);
    add_edge(11,v1,v4,sg);
    add_edge(12,v2,v3,sg);
    sg[v4] = 4;
    assert(num_vertices(sg)==4);
    assert(num_edges(sg)==5);
    commit(sg);
    sg[v4] = 5;
    remove_edge(v1,v4,sg);
    assert(num_edges(sg)==4);
    assert(!edge(v1,v4,sg).second);
    undo(sg);
    FILE_LOG(logDEBUG1) << "made undo";
    assert(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    assert(num_edges(sg)==5);
    FILE_LOG(logDEBUG1) << "count match";
    assert(sg[v4]==4);
    FILE_LOG(logDEBUG1) << "attribute match";

    typedef boost::graph_traits<simple_graph>::vertex_iterator vertex_iterator;

    //Tried to make this section more clear, instead of using tie, keeping all
    //the original types so it's more clear what is going on
    std::pair<vertex_iterator, vertex_iterator> vi = vertices(sg);
    for(vertex_iterator vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
        std::cout << "(" << *vertex_iter << ")\n";
    }

    typedef boost::graph_traits<simple_graph>::edge_iterator edge_iterator;

    //Tried to make this section more clear, instead of using tie, keeping all
    //the original types so it's more clear what is going on
    std::pair<edge_iterator, edge_iterator> ei = edges(sg);
    for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        std::cout << "(" << source(*edge_iter, sg) << ", " << target(*edge_iter, sg) << ")\n";
    }

    // Perform a topological sort.
    std::deque<int> topo_order;
    boost::topological_sort(sg, std::front_inserter(topo_order));
    // Print the results.
    for(std::deque<int>::const_iterator i = topo_order.begin();i != topo_order.end();++i)
    {
        std::cout << *i << std::endl;
    }

}

/*
void test1(){
    using namespace boost;
    using containerS = vecS;
    typedef subgraph< adjacency_list<containerS, containerS, directedS,
            property<vertex_index_t, size_t>,
            property<edge_index_t, size_t>
        >
    > Graph;

    const int N = 6;
    Graph G0(N);
    enum { A, B, C, D, E, F};

    Graph& G1 = G0.create_subgraph();
    Graph::vertex_descriptor CG1 = add_vertex(C,G1);
    Graph::vertex_descriptor EG1 = add_vertex(D,G1);

    add_edge(CG1, EG1, G1);

    print_graph(G0);
    std::cout << "SUBGRAPH:\n";
    print_graph(G1);
}

void test2(){
    using namespace boost;
      typedef subgraph< adjacency_list<vecS, vecS, directedS,
        property<vertex_color_t, int>, property<edge_index_t, int> > > Graph;

      const int N = 6;
      Graph G0(N);
      enum { A, B, C, D, E, F};     // for conveniently refering to vertices in G0

      Graph& G1 = G0.create_subgraph();
      Graph& G2 = G0.create_subgraph();
      enum { A1, B1, C1 };          // for conveniently refering to vertices in G1
      enum { A2, B2 };              // for conveniently refering to vertices in G2

      add_vertex(C, G1); // global vertex C becomes local A1 for G1
      add_vertex(E, G1); // global vertex E becomes local B1 for G1
      add_vertex(F, G1); // global vertex F becomes local C1 for G1

      add_vertex(A, G2); // global vertex A becomes local A1 for G2
      add_vertex(B, G2); // global vertex B becomes local B1 for G2

      add_edge(A, B, G0);
      add_edge(B, C, G0);
      add_edge(B, D, G0);
      add_edge(E, B, G0);
      add_edge(E, F, G0);
      add_edge(F, D, G0);

      add_edge(A1, C1, G1); // (A1,C1) is subgraph G1 local indices for (C,F).

      std::cout << "G0:" << std::endl;
      print_graph(G0, get(vertex_index, G0));
      print_edges2(G0, get(vertex_index, G0), get(edge_index, G0));
      std::cout << std::endl;

      Graph::children_iterator ci, ci_end;
      int num = 1;
      for (boost::tie(ci, ci_end) = G0.children(); ci != ci_end; ++ci) {
        std::cout << "G" << num++ << ":" << std::endl;
        print_graph(*ci, get(vertex_index, *ci));
        print_edges2(*ci, get(vertex_index, *ci), get(edge_index, *ci));
        std::cout << std::endl;
      }
}
*/

/*
void test3(){
    using namespace boost;
    using containerS = vecS;
    typedef subgraph< adjacency_list<containerS, containerS, directedS> > Graph;

    const int N = 6;
    Graph G0(N);
    enum { A, B, C, D, E, F};

    Graph& G1 = G0.create_subgraph();
    Graph::vertex_descriptor CG1 = add_vertex(C,G1);
    Graph::vertex_descriptor EG1 = add_vertex(D,G1);

    add_edge(CG1, EG1, G1);

    print_graph(G0);
    std::cout << "SUBGRAPH:\n";
    print_graph(G1);
}
*/
int main()
{
    test_graph();
//    test1();
//    test2();
}
