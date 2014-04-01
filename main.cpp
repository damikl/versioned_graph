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
    template<typename Graph, typename descriptor>
    static void print_vertices(const Graph& graph, descriptor v){
            graph[v].print();
    }
    template<typename Graph, typename descriptor>
    static void print_edges(const Graph& graph, descriptor e){
            std::cout << " data: ";
            graph[e].print();
            cout << " ";
    }

};
template<>
struct edge_printer<no_property>{
    template<typename Graph, typename descriptor>
    static void print_edges(const Graph& ,descriptor ){
    }
    template<typename Graph, typename descriptor>
    static void print_vertices(const Graph& ,descriptor){
    }
};


struct Printer{
    template<typename Graph>
    static void print_graph(const Graph& graph){

        print_edges(graph, edges(graph).first, edges(graph).second);
        print_vertices(graph, vertices(graph).first, vertices(graph).second);
    }
private:
    template<typename Graph,typename iterator>
    static void print_edges(const Graph& graph,iterator begin, iterator end){
        for(iterator edge_iter = begin; edge_iter != end; ++edge_iter) {
            int u = source(*edge_iter, graph);
            int v = target(*edge_iter, graph);
            std::cout << "(" << u << ", " << v << ")";

            std::cout << " from: ";
            edge_printer<typename vertex_bundle_type<Graph>::type>::print_vertices(graph, u);
            cout << " to: ";
            edge_printer<typename vertex_bundle_type<Graph>::type>::print_vertices(graph, v);
            cout << " ";
            edge_printer<typename edge_bundle_type<Graph>::type>::print_edges(graph, *edge_iter);
            cout << endl;
        }
    }
    template<typename Graph,typename iterator>
    static void print_vertices(const Graph&,iterator begin, iterator end){
        for(iterator vertex_iter = begin; vertex_iter != end; ++vertex_iter) {
            std::cout << "vertex(" << *vertex_iter << ")";

            cout << endl;
        }
    }

};

int main()
{
    FILELog::ReportingLevel() = logERROR;
    FILE* log_fd = fopen( "mylogfile.txt", "w" );
    Output2FILE::Stream() = log_fd;
    //create an -undirected- graph type, using vectors as the underlying containers
    //and an adjacency_list as the basic representation
    typedef boost::adjacency_list<vecS, vecS, undirectedS, extra_info> UndirectedGraph;
    typedef boost::adjacency_list<vecS, vecS, undirectedS> SimpleGraph;
    typedef boost::adjacency_list<vecS, vecS, undirectedS, extra_info,extra_info> ExtendedGraph;

    //Our set of edges, which basically are just converted into ints (0-4)
    enum {A, B, C, D, E, N};

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
    cout << "Graph with data on vertices" << endl;
    //Now we can initialize our graph using iterators from our above vector
    UndirectedGraph g(edgeVec.begin(), edgeVec.end(), N);

    add_edge(A, E, g);

    UndirectedGraph back = g;
    graph_archive<UndirectedGraph> arch(g);
    arch.commit();
    //Finally lets add a new vertex - remember the verticies are just of type int
    int F = add_vertex(g);

    //Connect our new vertex with an edge to A...
    add_edge(A, F, g);
    remove_edge(B, D, g);

    g[F].simple_name = "other";
    g[D].simple_name = "just something";

    arch.commit();
    UndirectedGraph ng = arch.checkout(1);
    bool first = equal(back,ng);
    if(first){
        std::cout << "versions are equal" << std::endl;
    } else{
        std::cout << "versions not equal" << std::endl;
        Printer::print_graph(back);
        std::cout << "verus" << std::endl;
        Printer::print_graph(ng);
        return -1;
    }
    bool second = equal(arch.checkout(2),g);
    if(second){
        std::cout << "versions are equal" << std::endl;
    } else{
        std::cout << "versions not equal" << std::endl;
        Printer::print_graph(g);
        std::cout << "verus" << std::endl;
        Printer::print_graph(arch.checkout(2));
        return -1;
    }
    FILELog::ReportingLevel() = logDEBUG4;
    cout << endl << "Graph with no extra data" << endl;
    FILE_LOG(logDEBUG1) << "Graph with no extra data";
    SimpleGraph simple(edgeVec.begin(), edgeVec.end(), N);
    SimpleGraph s_copy = simple;
    graph_archive<SimpleGraph> simple_arch(simple);
    simple_arch.commit();
    add_edge(A, F, simple);
    remove_edge(B, D, simple);
    cout << "added vertex: " << add_vertex(simple) << endl;
    simple_arch.commit();

    first = equal(s_copy,simple_arch.checkout(1));
    if(first){
        std::cout << "versions are equal" << std::endl;
    } else{
        std::cout << "versions not equal" << std::endl;
        Printer::print_graph(s_copy);
        std::cout << "verus" << std::endl;
        Printer::print_graph(simple_arch.checkout(1));
        return -1;
    }
    second = equal(simple_arch.checkout(2),simple);
    if(second){
        std::cout << "versions are equal" << std::endl;
    } else{
        std::cout << "versions not equal" << std::endl;
        Printer::print_graph(simple);
        std::cout << "verus" << std::endl;
        Printer::print_graph(simple_arch.checkout(2));
        return -1;
    }

    cout << endl << "Graph with data on vertices and edges" << endl;

    ExtendedGraph ex(edgeVec.begin(), edgeVec.end(), N);
    ExtendedGraph ex_copy = ex;
    graph_archive<ExtendedGraph> ex_arch(ex);
    ex_arch.commit();
    pair<ExtendedGraph::edge_descriptor,bool> p = add_edge(A, F, ex);
    remove_edge(B, D, ex);
    ex[p.first].simple_name = "super edge";
    ex[F].simple_name = "other";
    ex[D].simple_name = "just something";
    cout << "added vertex: " << add_vertex(ex)<< endl;
    ex_arch.commit();
//    ex[A].simple_name = "aaa";     // breaks next comparison

    first = equal(ex_copy,ex_arch.checkout(1));
    if(first){
        std::cout << "versions are equal" << std::endl;
    } else{
        std::cout << "versions not equal" << std::endl;
        Printer::print_graph(ex_copy);
        std::cout << "verus" << std::endl;
        Printer::print_graph(ex_arch.checkout(1));
        return -1;
    }
    second = equal(ex_arch.checkout(2),ex);
    if(second){
        std::cout << "versions are equal" << std::endl;
    } else{
        std::cout << "versions not equal" << std::endl;
        Printer::print_graph(ex_arch.checkout(2));
        std::cout << "verus" << std::endl;
        Printer::print_graph(ex);
        return -1;
    }
    ex_arch.replace_with(1);
    first = equal(ex_copy,ex);
    if(first){
        std::cout << "versions are equal" << std::endl;
    } else{
        std::cout << "versions not equal" << std::endl;
        Printer::print_graph(ex_copy);
        std::cout << "verus" << std::endl;
        Printer::print_graph(ex);
        return -1;
    }

    return 0;
}

