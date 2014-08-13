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
        typedef typename graph_traits < Graph >::vertex_descriptor Vertex;
        for(iterator edge_iter = begin; edge_iter != end; ++edge_iter) {
            Vertex u = source(*edge_iter, graph);
            Vertex v = target(*edge_iter, graph);
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

template<typename Graph>
bool check_equality(const Graph& g1,const Graph& g2 ){
    if(equal(g1,g2)){
        std::cout << "versions are equal" << std::endl;
    } else{
        std::cout << "versions not equal" << std::endl;
        Printer::print_graph(g1);
        std::cout << "verus" << std::endl;
        Printer::print_graph(g2);
        exit(-1);
        return false;
    }
    return true;
}

template<typename Graph>
class graph_test{
public:
    typedef typename graph_traits < Graph >::vertex_descriptor Vertex;
    typedef typename graph_traits < Graph >::edge_descriptor Edge;
    graph_test() : archive(current),current() {
        fill_graph(current);
        commit();
    }
    static void fill_graph(Graph& g ){

        Vertex A,B,C,D,E;
        A = boost::add_vertex(g);
        B = boost::add_vertex(g);
        C = boost::add_vertex(g);
        D = boost::add_vertex(g);
        E = boost::add_vertex(g);
        boost::add_edge(A,B,g);
        boost::add_edge(A,D,g);
        boost::add_edge(C,A,g);
        boost::add_edge(D,C,g);
        boost::add_edge(C,E,g);
        boost::add_edge(B,D,g);
        boost::add_edge(D,E,g);
    }
    void add_edge(int u, int v){
        typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
        std::pair<vertex_iterator, vertex_iterator> vi = vertices(current);
        vertex_iterator it_u = vi.first;
        vertex_iterator it_v = vi.first;
        advance(it_u,u);
        advance(it_v,v);
        boost::add_edge(*it_u,*it_v,current);
    }
    void remove_edge(int u,int v){
        typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
        std::pair<vertex_iterator, vertex_iterator> vi = vertices(current);
        vertex_iterator it_u = vi.first;
        vertex_iterator it_v = vi.first;
        assert(std::distance(vi.first,vi.second) > u);
        advance(it_u,u);
        assert(std::distance(vi.first,vi.second) > v);
        advance(it_v,v);
        boost::remove_edge(*it_u,*it_v,current);
    }

    Graph& getGraph(){
        return current;
    }
    Vertex getVertex(int u){
        typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
        vertex_iterator it_u = vertices(current).first;
        std::advance(it_u,u);
        return *it_u;
    }

    Edge getEdge(int _u, int _v){
        Vertex u = getVertex(_u);
        Vertex v = getVertex(_v);
        return boost::edge(u,v,current).first;
    }

    Vertex add_vertex(){
        return boost::add_vertex(current);
    }
    void remove_vertex(int u){
        typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
        vertex_iterator it_u = vertices(current).first;
        advance(it_u,u);
        FILE_LOG(logDEBUG1) << "removed vertex " << *it_u;
        boost::clear_vertex(*it_u,current);
        boost::remove_vertex(*it_u,current);
    }
    void commit(){
        archive.commit();
        this->snapshots.insert(make_pair(archive.head_rev(),current));
    }
    void test(){
        archive.commit();
        for(int i = 1;i <5;++i)
            FILE_LOG(logDEBUG1) << "TEST: added "<< i << " vertex: " << add_vertex() << endl;

        add_edge(0, 5);
        commit();
        FILE_LOG(logDEBUG1) << "TEST: Check vertices count";
        int v_num = archive.num_vertices(1);
        FILE_LOG(logDEBUG1) << "TEST: Check vertices count is equal: " << v_num;
        assert(v_num==5);
        FILE_LOG(logDEBUG1) << "TEST: Check edges count";
        int e_num = archive.num_edges(1);
        FILE_LOG(logDEBUG1) << "TEST: Check edges count is equal: " << e_num;
        assert(e_num==7);
        check();
        remove_vertex(5);
        commit();
        check();
        FILE_LOG(logDEBUG1) << "test finished";
    }
    void check(){
        for (typename std::map<int,Graph>::iterator it=snapshots.begin(); it!=snapshots.end(); ++it){
            FILE_LOG(logDEBUG1) << "CHECK REVISION: " << it->first;
            check_equality(it->second,archive.checkout(it->first));
        }
    }

private:
    graph_archive<Graph> archive;
    map<int,Graph> snapshots;
    Graph current;
};


template<typename test_container>
void test_case1(test_container& t){

}


int main()
{
    FILELog::ReportingLevel() = logDEBUG4;
    FILE* log_fd = fopen( "mylogfile.txt", "w" );
    Output2FILE::Stream() = log_fd;
    //create an -undirected- graph type, using vectors as the underlying containers
    //and an adjacency_list as the basic representation
    typedef boost::adjacency_list<listS, listS, undirectedS, extra_info> UndirectedGraph;
    typedef boost::adjacency_list<listS, listS, undirectedS> SimpleGraph;
    typedef boost::adjacency_list<listS, listS, undirectedS, extra_info,extra_info> ExtendedGraph;

    cout << endl << "Graph with data on vertices" << endl;
    FILE_LOG(logDEBUG1) << "Graph with data on vertices";
    graph_test<UndirectedGraph> g;
    g.test();

    g.getGraph()[g.getVertex(5)].simple_name = "other";
    g.getGraph()[g.getVertex(3)].simple_name = "just something";

    g.commit();

    g.check();

    FILELog::ReportingLevel() = logDEBUG4;
    cout << endl << "Graph with no extra data" << endl;
    FILE_LOG(logDEBUG1) << "Graph with no extra data";
    graph_test<SimpleGraph> simple;
    simple.test();
    simple.check();

    cout << endl << "Graph with data on vertices and edges" << endl;

    graph_test<ExtendedGraph> ex;
    ex.test();

    ex.getGraph()[ex.getVertex(5)].simple_name = "other";
    ex.getGraph()[ex.getVertex(3)].simple_name = "just something";
    ex.getGraph()[ex.getEdge(0,3)].simple_name = "path weight";
    ex.commit();

    ex.check();

    return 0;
}

