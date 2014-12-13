#ifndef TESTS_H
#define TESTS_H

#include <iostream>
#include <vector>
#include "archive.h"
#include "gtest/gtest.h"

struct extra_info {
    std::string simple_name;
    int othervalue;
    std::vector<int> some_values;
    bool operator ==(const extra_info& vertex)const{
        return this->simple_name == vertex.simple_name;
    }
    bool operator !=(const extra_info& vertex)const{
        return this->simple_name != vertex.simple_name;
    }
    std::string to_string() const{
        return simple_name + std::string(" ");
    }
};

template<typename properties_type>
struct edge_printer{
    template<typename Graph, typename descriptor>
    static std::string print_vertices(const Graph& graph, descriptor v){
            return graph[v].to_string();
    }
    template<typename Graph, typename descriptor>
    static std::string print_edges(const Graph& graph, descriptor e){
            return std::string(" data: ") + graph[e].to_string();
    }

};
template<>
struct edge_printer<boost::no_property>{
    template<typename Graph, typename descriptor>
    static std::string print_edges(const Graph& ,descriptor ){
        return std::string();
    }
    template<typename Graph, typename descriptor>
    static std::string print_vertices(const Graph& ,descriptor){
        return std::string();
    }
};


struct Printer{
    template<typename Graph>
    static void print_graph(const Graph& graph){

        std::cout << print_edges(graph, edges(graph).first, edges(graph).second) << std::endl;
        std::cout << print_vertices(graph, vertices(graph).first, vertices(graph).second) << std::endl;
    }
private:
    template<typename Graph,typename iterator>
    static std::string print_edges(const Graph& graph,iterator begin, iterator end){
        typedef typename boost::graph_traits < Graph >::vertex_descriptor Vertex;
        std::stringstream ss;
        for(iterator edge_iter = begin; edge_iter != end; ++edge_iter) {
            Vertex u = source(*edge_iter, graph);
            Vertex v = target(*edge_iter, graph);
            ss << "(" << u << ", " << v << ")";

            ss << " from: ";
            ss << edge_printer<typename boost::vertex_bundle_type<Graph>::type>::print_vertices(graph, u);
            ss << " to: ";
            ss << edge_printer<typename boost::vertex_bundle_type<Graph>::type>::print_vertices(graph, v);
            ss << " ";
            ss << edge_printer<typename boost::edge_bundle_type<Graph>::type>::print_edges(graph, *edge_iter);
            ss << std::endl;
        }
        return ss.str();
    }
    template<typename Graph,typename iterator>
    static std::string print_vertices(const Graph&,iterator begin, iterator end){
        std::stringstream ss;
        for(iterator vertex_iter = begin; vertex_iter != end; ++vertex_iter) {
            ss << "vertex(" << *vertex_iter << ")";

            ss << std::endl;
        }
        return ss.str();
    }

};

template<typename Graph>
class GraphTest : public testing::Test {
public:
    typedef typename boost::graph_traits < Graph >::vertex_descriptor Vertex;
    typedef typename boost::graph_traits < Graph >::edge_descriptor Edge;
    typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
    GraphTest() : archive(current),current() {
        fill_graph();
        commit();
    }
    virtual void fill_graph(){

        Vertex A,B,C,D,E;
        A = boost::add_vertex(current);
        B = boost::add_vertex(current);
        C = boost::add_vertex(current);
        D = boost::add_vertex(current);
        E = boost::add_vertex(current);
        boost::add_edge(A,B,current);
        boost::add_edge(A,D,current);
        boost::add_edge(C,A,current);
        boost::add_edge(D,C,current);
        boost::add_edge(C,E,current);
        boost::add_edge(B,D,current);
        boost::add_edge(D,E,current);
    }
    void add_edge(int u, int v){
        vertex_iterator it_u = getVertexIterator(u);
        vertex_iterator it_v = getVertexIterator(v);
        boost::add_edge(*it_u,*it_v,current);
    }
    void remove_edge(int u,int v){
        vertex_iterator it_u = getVertexIterator(u);
        vertex_iterator it_v = getVertexIterator(v);
        boost::remove_edge(*it_u,*it_v,current);
    }

    Graph& getGraph(){
        return current;
    }
    Vertex getVertex(int u){
        return *getVertexIterator(u);
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
        vertex_iterator it_u = getVertexIterator(u);
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
            FILE_LOG(logDEBUG1) << "TEST: added "<< i << " vertex: " << add_vertex() << std::endl;

        add_edge(0, 5);
        commit();
        FILE_LOG(logDEBUG1) << "TEST: Check vertices count";
        ASSERT_EQ(archive.num_vertices(1),5);
        ASSERT_EQ(archive.num_edges(1),7);
        check();
    }
    void test_removal(){
        remove_vertex(5);
        commit();
        check();
    }
    bool check_equality(const Graph& g1,const Graph& g2 ){
        bool res = equal(g1,g2);
        if(res){
            std::cout << "versions are equal" << std::endl;
        } else{
            std::cout << "versions not equal" << std::endl;
            Printer::print_graph(g1);
            std::cout << "verus" << std::endl;
            Printer::print_graph(g2);
        //    EXPECT_TRUE(res);
            return false;
        }
        return true;
    }
    bool check(){
        for (typename std::map<int,Graph>::iterator it=snapshots.begin(); it!=snapshots.end(); ++it){
            FILE_LOG(logDEBUG1) << "CHECK REVISION: " << it->first;
            if(!check_equality(it->second,archive.checkout(it->first))){
                return false;
            }
        }
        return true;
    }

protected:

    vertex_iterator getVertexIterator(int u){
        std::pair<vertex_iterator, vertex_iterator> vi = vertices(current);
        assert(std::distance(vi.first,vi.second) > u);
        vertex_iterator it_u = vi.first;
        std::advance(it_u,u);
        return it_u;
    }

    graph_archive<Graph> archive;
    std::map<int,Graph> snapshots;
    Graph current;
};




#endif // TESTS_H
