#include <iostream>
#include <vector>
#include "archive.h"

using namespace std;
using namespace boost;

struct extra_info {
    string simple_name;
    int othervalue;
    vector<int> some_values;
    bool operator ==(const extra_info& vertex)const{
        return this->simple_name == vertex.simple_name;
    }
    bool operator !=(const extra_info& vertex)const{
        return this->simple_name != vertex.simple_name;
    }
    void print() const{
        cout << simple_name << " ";
    }
};

template<typename properties_type>
struct edge_printer{

    template<typename Graph, typename iterator>
    static void print_edges(const Graph& graph, iterator begin, iterator end){
        for(iterator edge_iter = begin; edge_iter != end; ++edge_iter) {
            int u = source(*edge_iter, graph);
            int v = target(*edge_iter, graph);
            std::cout << "(" << u << ", " << v << ")" << " from: ";
            graph[u].print();
            cout << " to: ";
            graph[v].print();
            cout << endl;
        }
    }
};
template<>
struct edge_printer<no_property>{
    template<typename Graph, typename iterator>
    static void print_edges(const Graph& graph,iterator begin, iterator end){
        for(iterator edge_iter = begin; edge_iter != end; ++edge_iter) {
            int u = source(*edge_iter, graph);
            int v = target(*edge_iter, graph);
            std::cout << "(" << u << ", " << v << ")" << endl;
        }
    }
};

template<typename Graph>
struct Printer{
    static void print_edges(const Graph& graph){
        edge_printer<typename vertex_bundle_type<Graph>::type>::print_edges(graph, edges(graph).first, edges(graph).second);
    }

};

int main()
{
    //create an -undirected- graph type, using vectors as the underlying containers
    //and an adjacency_list as the basic representation
    typedef boost::adjacency_list<vecS, vecS, undirectedS, extra_info> UndirectedGraph;
    typedef boost::adjacency_list<vecS, vecS, undirectedS> SimpleGraph;
    typedef boost::adjacency_list<vecS, vecS, undirectedS, extra_info,extra_info> ExtendedGraph;

    //Our set of edges, which basically are just converted into ints (0-4)
    enum {A, B, C, D, E, N};
    const char *name = "ABCDE";

    //An edge is just a connection between two vertitices. Our verticies above
    //are an enum, and are just used as integers, so our edges just become
    //a std::pair<int, int>
    typedef std::pair<int, int> Edge;

    //Example uses an array, but we can easily use another container type
    //to hold our edges.
    std::vector<Edge> edgeVec;
    edgeVec.push_back(Edge(A,B));
    edgeVec.push_back(Edge(A,D));
    edgeVec.push_back(Edge(C,A));
    edgeVec.push_back(Edge(D,C));
    edgeVec.push_back(Edge(C,E));
    edgeVec.push_back(Edge(B,D));
    edgeVec.push_back(Edge(D,E));

    //Now we can initialize our graph using iterators from our above vector
    UndirectedGraph g(edgeVec.begin(), edgeVec.end(), N);

    std::cout << num_edges(g) << "\n";
    g[D].simple_name = "Alpha";

    std::cout << "HHH " << typeid(g[D]).name() << std::endl;
    //Ok, we want to see that all our edges are now contained in the graph
    typedef graph_traits<UndirectedGraph>::edge_iterator edge_iterator;

    //Tried to make this section more clear, instead of using tie, keeping all
    //the original types so it's more clear what is going on
    Printer<UndirectedGraph>::print_edges(g);

    std::cout << "Want to add another edge between (A,E)\n";
    //Want to add another edge between (A,E)?
    add_edge(A, E, g);

    //Print out the edge list again to see that it has been added
    Printer<UndirectedGraph>::print_edges(g);

    UndirectedGraph back = g;
    graph_archive<UndirectedGraph> arch(g);
    arch.commit();
    //Finally lets add a new vertex - remember the verticies are just of type int
    int F = add_vertex(g);
    std::cout << "new vertex: " << F << "\n";

    //Connect our new vertex with an edge to A...
    add_edge(A, F, g);
    remove_edge(B, D, g);
    std::cout << "removed edge: " << B <<" to " << D << "\n";
    g[F].simple_name = "other";
    g[D].simple_name = "just something";
    Printer<UndirectedGraph>::print_edges(g);

    arch.commit();
    std::cout << "before checkout\n";
    UndirectedGraph ng = arch.checkout(1);
    bool first = equal(back,ng);
    bool second = equal(arch.checkout(2),g);
    std::cout << "Comparison: " << first << " " << second << std::endl;

    Printer<UndirectedGraph>::print_edges(g);

    SimpleGraph simple(edgeVec.begin(), edgeVec.end(), N);
    SimpleGraph s_copy = simple;
    graph_archive<SimpleGraph> simple_arch(simple);
    simple_arch.commit();
    add_edge(A, F, simple);
    remove_edge(B, D, simple);
    simple_arch.commit();

    first = equal(s_copy,simple_arch.checkout(1));
    second = equal(simple_arch.checkout(2),simple);
    std::cout << "Comparison: " << first << " " << second << std::endl;
    Printer<SimpleGraph>::print_edges(simple);

    ExtendedGraph ex(edgeVec.begin(), edgeVec.end(), N);
    ExtendedGraph ex_copy = ex;
    graph_archive<ExtendedGraph> ex_arch(ex);
    ex_arch.commit();
    add_edge(A, F, ex);
    remove_edge(B, D, ex);
    ex[F].simple_name = "other";
    ex[D].simple_name = "just something";
    ex_arch.commit();
//    ex[A].simple_name = "aaa";     // breaks next comparison

    first = equal(ex_copy,ex_arch.checkout(1));
    second = equal(ex_arch.checkout(2),ex);
    std::cout << "Comparison: " << first << " " << second << std::endl;
    Printer<ExtendedGraph>::print_edges(ex);


    return 0;
}

