#include <iostream>
#include <vector>
#include "archive.h"

using namespace std;
using namespace boost;

struct vertex_info {
    string simple_name;
    int othervalue;
    vector<int> some_values;
    bool operator ==(const vertex_info& vertex)const{
        return this->simple_name == vertex.simple_name;
    }
    bool operator !=(const vertex_info& vertex)const{
        return this->simple_name != vertex.simple_name;
    }
};



int main()
{
    //create an -undirected- graph type, using vectors as the underlying containers
    //and an adjacency_list as the basic representation
    typedef boost::adjacency_list<vecS, vecS, undirectedS, vertex_info> UndirectedGraph;

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
    std::pair<edge_iterator, edge_iterator> ei = edges(g);
    for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        std::cout << "(" << source(*edge_iter, g) << ", " << target(*edge_iter, g) << ")\n";
    }

    std::cout << "Want to add another edge between (A,E)\n";
    //Want to add another edge between (A,E)?
    add_edge(A, E, g);

    //Print out the edge list again to see that it has been added
    for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        std::cout << "(" << source(*edge_iter, g) << ", " << target(*edge_iter, g) << ")\n";
    }
    UndirectedGraph back = g;
    graph_archive<UndirectedGraph> arch(g);
    arch.commit(ei);
    //Finally lets add a new vertex - remember the verticies are just of type int
    int F = add_vertex(g);
    std::cout << "new vertex: " << F << "\n";

    //Connect our new vertex with an edge to A...
    add_edge(A, F, g);
    for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        std::cout << "(" << source(*edge_iter, g) << ", " << target(*edge_iter, g) << ")\n";
    }
    arch.commit(ei);
    std::cout << "before checkout\n";
    UndirectedGraph ng = arch.checkout(1);

    std::cout << "hello"  << ng[D].simple_name << equal(back,ng) << std::endl;

    std::pair<edge_iterator, edge_iterator> bi = edges(back);
    for(edge_iterator edge_iter = bi.first; edge_iter != bi.second; ++edge_iter) {
        std::cout << "(" << source(*edge_iter, back) << ", " << target(*edge_iter, back) << ")\n";
    }

    UndirectedGraph::vertex_descriptor u,v;
    std::pair<edge_iterator, edge_iterator> ngi = edges(ng);
    for(edge_iterator edge_iter = ngi.first; edge_iter != ngi.second; ++edge_iter) {
        u = source(*edge_iter, ng);
        v = target(*edge_iter, ng);
        std::cout << "(" << u << ", " << v << ")" << " from: "<< ng[u].simple_name << " to: " << ng[v].simple_name  << "\n";
    }

    return 0;
}

