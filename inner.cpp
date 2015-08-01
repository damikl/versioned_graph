//#include <boost/config.hpp>
#include <iostream>
#include <utility>
//#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include "utils.h"
#include <boost/iterator/filter_iterator.hpp>
#include <boost/graph/topological_sort.hpp>
#include "versioned_graph.h"

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
    undo_commit(sg);
    FILE_LOG(logDEBUG1) << "made undo";
    assert(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    assert(num_edges(sg)==5);
    FILE_LOG(logDEBUG1) << "count match";
    assert(sg[v4]==4);
    FILE_LOG(logDEBUG1) << "attribute match";
    auto v5 = add_vertex(5,sg);
    add_edge(13,v5,v4,sg);
    add_edge(14,v3,v5,sg);
    assert(num_vertices(sg)==5);
    assert(num_edges(sg)==7);
    assert(sg[v5]==5);


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
