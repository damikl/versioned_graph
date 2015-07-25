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
    const graph_type& g;
//    typedef typename value_type entry;
public:
    filter_predicate(const filter_predicate &p) : rev(p.rev),g(p.g){}
    filter_predicate(const graph_type& graph,const revision& r = std::numeric_limits<int>::max()) : rev(r),g(graph) {}
    bool operator()(const value_type& v) {
        auto list = g[v];
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


    typedef typename boost::filter_iterator<
                                    filter_predicate<self,vertex_descriptor>,
                                                     typename boost::graph_traits<graph_type>::vertex_iterator>
                                                                                        vertex_iterator;
    typedef typename boost::filter_iterator<
                                    filter_predicate<self,edge_descriptor>,
                                                     typename boost::graph_traits<graph_type>::edge_iterator>
                                                                                        edge_iterator;



    auto vertices_begin(){
        return typename graph_type::vertex_iterator(this->vertex_set().begin());
    }
    auto vertices_end(){
        return typename graph_type::vertex_iterator(this->vertex_set().end());
    }

    auto edges_begin(){
        return typename graph_type::edge_iterator(this->m_edges.begin());
    }
    auto edges_end(){
        return typename graph_type::edge_iterator(this->m_edges.end());
    }

    versioned_graph() : /*g(),*/current_rev(1),v_num(0),e_num(0) {}
    vertex_descriptor generate_vertex(VertexProperties prop){
        vertex_descriptor v = boost::add_vertex(*this);
        vertices_history_type& list = (*this)[v];
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
            edges_history_type& list = (*this)[p.first];
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
    void set_deleted(edge_descriptor e){
        edges_history_type& list = (*this)[e];
        FILE_LOG(logDEBUG4) << "set deleted: " << list.size() << " records in history";
        assert(!is_deleted(get_revision(list.front())));
        revision r = make_deleted(current_rev);
        FILE_LOG(logDEBUG4) << "negated to " << r;
        typename edges_history_type::value_type p = std::make_pair(r,edge_bundled());
        FILE_LOG(logDEBUG4) << "entry created";
        list.push_front(p);
        --e_num;
        assert(is_deleted(get_revision((*this)[e].front())));
        FILE_LOG(logDEBUG4) << "set deleted: finnished";
    }
    void set_deleted(vertex_descriptor v){
        vertices_history_type& list = (*this)[v];
        FILE_LOG(logDEBUG4) << "set deleted: " << list.size() << " records in history";
        assert(!is_deleted(get_revision(list.front())));
        revision r = make_deleted(current_rev);
        FILE_LOG(logDEBUG4) << "negated to " << r;
        typename vertices_history_type::value_type p = std::make_pair(r,edge_bundled());
        FILE_LOG(logDEBUG4) << "entry created";
        list.push_front(p);
        --v_num;
        assert(is_deleted(get_revision((*this)[v].front())));
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

    filter_predicate<graph_type,typename graph_type::vertex_descriptor> predicate(g);
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

    filter_predicate<graph_type,typename graph_type::edge_descriptor> predicate(g);
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
        FILE_LOG(logDEBUG4) << "remove_edge: got desc";
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
    auto p = edge(u,v,dynamic_cast<const typename graph_type::graph_type&>(g));
    if(p.second){
        FILE_LOG(logDEBUG4) << "edge: found existing intenally";
        auto edge_desc = p.first;
        const typename graph_type::edges_history_type& list = g[edge_desc];
        FILE_LOG(logDEBUG4) << /*list.size()  <<*/ "records in history of edge";
        if(is_deleted(get_revision(list.front()))){
            FILE_LOG(logDEBUG4) << "edge: is deleted";
            return std::make_pair(typename graph_type::edge_descriptor(),false);
        }
        FILE_LOG(logDEBUG4) << "edge: not deleted";
    }
    return p;
}




void test_graph(){
    using namespace boost;
    using namespace std;
    typedef versioned_graph<vecS, vecS, undirectedS,int,int> simple_graph;
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
    add_edge(11,v4,v1,sg);
    add_edge(12,v3,v2,sg);
    assert(num_vertices(sg)==4);
    assert(num_edges(sg)==5);
    commit(sg);
    remove_edge(v4,v1,sg);
    assert(num_edges(sg)==4);
    assert(!edge(v4,v1,sg).second);
    /*
    // Perform a topological sort.
    std::deque<int> topo_order;
    boost::topological_sort(sg, std::front_inserter(topo_order));
    // Print the results.
    for(std::deque<int>::const_iterator i = topo_order.begin();i != topo_order.end();++i)
    {
        std::cout << *i << std::endl;
    }
    */

    typedef graph_traits<simple_graph>::vertex_iterator vertex_iterator;

    //Tried to make this section more clear, instead of using tie, keeping all
    //the original types so it's more clear what is going on
    std::pair<vertex_iterator, vertex_iterator> vi = vertices(sg);
    for(vertex_iterator vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
        std::cout << "(" << *vertex_iter << ")\n";
    }

    typedef graph_traits<simple_graph>::edge_iterator edge_iterator;

    //Tried to make this section more clear, instead of using tie, keeping all
    //the original types so it's more clear what is going on
    std::pair<edge_iterator, edge_iterator> ei = edges(sg);
    for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        std::cout << "(" << source(*edge_iter, sg) << ", " << target(*edge_iter, sg) << ")\n";
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
