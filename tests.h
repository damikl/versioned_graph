#ifndef TESTS_H
#define TESTS_H

#include <iostream>
#include <vector>
#include <memory>
#include "archive.h"
#include "isomorphism_checking.h"
#include "handler.h"

#include "gtest/gtest.h"
#include "boost/graph/copy.hpp"


template<typename Graph>
Graph clone_graph(const Graph& g){
    typedef typename boost::graph_traits < Graph >::vertex_descriptor Vertex;
    typedef typename boost::graph_traits<Graph>::edge_iterator edge_iterator;
    typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
    typedef typename boost::vertex_bundle_type<Graph>::type vertex_property;
    typedef typename boost::edge_bundle_type<Graph>::type edge_property;
    std::map<Vertex,Vertex> v_map;
    Graph clone;
    std::pair<vertex_iterator, vertex_iterator> v_g1 = boost::vertices(g);
    for(vertex_iterator it = v_g1.first; it!=v_g1.second;++it){
        vertex_property p = g[*it];
        Vertex v = boost::add_vertex(p,clone);
        v_map.insert(std::make_pair(*it,v));
        FILE_LOG(logDEBUG4) << "cloned vertex " << *it << " as " << v;
    }
    assert (boost::num_vertices(clone) == boost::num_vertices(g));
    FILE_LOG(logDEBUG3) << "cloned vertices";
    std::pair<edge_iterator, edge_iterator> e_g1 = boost::edges(g);
    for(edge_iterator it = e_g1.first; it!=e_g1.second;++it){
        Vertex oryg_source = boost::source(*it,g);
        FILE_LOG(logDEBUG4) << "find vertex for " << oryg_source;
        auto result = v_map.find(oryg_source);
        assert (result != v_map.end());
        Vertex s = result->second;
        result = v_map.find(boost::target(*it,g));
        assert (result != v_map.end());
        Vertex t = result->second;
        edge_property p = g[*it];
        boost::add_edge(s,t,p,clone);
    //    clone[e] = g[*it];
    }
    assert (boost::num_edges(clone) == boost::num_edges(g));
    FILE_LOG(logDEBUG3) << "cloned edges";
    return clone;

}

struct extra_info {
private:
    static int counter;
public:
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
        return simple_name + std::string(" ") + std::to_string(othervalue) + std::string(" ");
    }
    extra_info() : othervalue(counter){
        ++counter;
    }
};
int extra_info::counter = 0;

namespace std{
    template<>
    struct hash<extra_info>
    {
        typedef extra_info argument_type;
        typedef size_t result_type;

        result_type operator()(argument_type const& s) const
        {
            return hash<std::string>()(s.simple_name)
                    ^ boost::hash<std::vector<int> >()(s.some_values)
                    ^ hash<int>()(s.othervalue);
        }
    };
    template<>
    struct hash<boost::no_property>
    {
        typedef boost::no_property argument_type;
        typedef size_t result_type;

        result_type operator()(argument_type const&) const
        {
            return hash<int>()(0);
        }
    };
}

std::ostream& operator<< (std::ostream& stream, const extra_info& info){
    stream << info.to_string() << &info << " ";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const boost::no_property& ){
    stream << " no_property ";
    return stream;
}

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
    typedef archive_handle<Graph> handle_type;

    GraphTest() : handle(archive),archive(),snapshots() {
        FILE* log_fd = fopen( "mylogfile.txt", "w" );
        Output2FILE::Stream() = log_fd;
        FILELog::ReportingLevel() = logDEBUG4;
        fill_graph();
     //   handle_type h = ::commit(archive,current);
     //   handle(new archive_handle<Graph>(h));
    //    handle.commit();
        commit();
    }
    virtual void fill_graph(){

        Vertex A,B,C,D,E;
        A = boost::add_vertex(handle.getGraph());
        B = boost::add_vertex(handle.getGraph());
        C = boost::add_vertex(handle.getGraph());
        D = boost::add_vertex(handle.getGraph());
        E = boost::add_vertex(handle.getGraph());
        boost::add_edge(A,B,handle.getGraph());
        boost::add_edge(A,D,handle.getGraph());
        boost::add_edge(C,A,handle.getGraph());
        boost::add_edge(D,C,handle.getGraph());
        boost::add_edge(C,E,handle.getGraph());
        boost::add_edge(B,D,handle.getGraph());
        boost::add_edge(D,E,handle.getGraph());
        EXPECT_EQ(5,boost::num_vertices(handle.getGraph()));
        EXPECT_EQ(7,boost::num_edges(handle.getGraph()));
    }
    void add_edge(int u, int v){
        vertex_iterator it_u = getVertexIterator(u);
        vertex_iterator it_v = getVertexIterator(v);
        boost::add_edge(*it_u,*it_v,handle.getGraph());
    }
    void remove_edge(int u,int v){
        vertex_iterator it_u = getVertexIterator(u);
        vertex_iterator it_v = getVertexIterator(v);
        boost::remove_edge(*it_u,*it_v,handle.getGraph());
    }

    Graph& getGraph(){
        return handle.getGraph();
    }
    Vertex getVertex(int u){
        return *getVertexIterator(u);
    }

    Edge getEdge(int _u, int _v){
        Vertex u = getVertex(_u);
        Vertex v = getVertex(_v);
        return boost::edge(u,v,handle.getGraph()).first;
    }

    Vertex add_vertex(){
        return boost::add_vertex(handle.getGraph());
    }
    void remove_vertex(int u){
        int count = boost::num_vertices(handle.getGraph());
        vertex_iterator it_u = getVertexIterator(u);
        FILE_LOG(logDEBUG1) << "removed vertex " << *it_u;
        boost::clear_vertex(*it_u,handle.getGraph());
        boost::remove_vertex(*it_u,handle.getGraph());
        ASSERT_EQ(count-1,boost::num_vertices(handle.getGraph()));
    }
    void commit(){
        FILE_LOG(logDEBUG4) << "TEST COMMIT " << archive.head_rev();
        auto old_rev = handle.get_revision();
        ::commit(handle);
        ASSERT_LT(old_rev,handle.get_revision());
        FILE_LOG(logDEBUG4) << "COMMITED " << archive.head_rev();
        Graph g_temp = clone_graph(handle.getGraph());
        //boost::copy_graph(handle.getGraph(),g_temp);
        ASSERT_TRUE(check_equality(handle.getGraph(),g_temp));
        this->snapshots.insert(std::make_pair(archive.head_rev(),g_temp));
        FILE_LOG(logDEBUG4) << "TEST COMMIT " << archive.head_rev() << " ENDED";
    }
    void test(){
        check();
        ASSERT_EQ(archive.head_rev(),revision(1));
        for(int i = 1;i <5;++i)
            FILE_LOG(logDEBUG1) << "TEST: added "<< i << " vertex: " << add_vertex() << std::endl;

        add_edge(0, 5);
        ASSERT_EQ(archive.num_vertices(1),5);
        ASSERT_EQ(archive.num_edges(1),7);
        ASSERT_FALSE(same_as_head());
        commit();
        FILE_LOG(logDEBUG1) << "TEST: Check vertices count";
        ASSERT_EQ(archive.num_vertices(1),5);
        ASSERT_EQ(archive.num_edges(1),7);
        check();
        ASSERT_EQ(archive.head_rev(),revision(2));
    }
    void test_removal(){
        remove_vertex(5);
        ASSERT_FALSE(same_as_head());
        commit();
    }
    bool check_equality(const Graph& g1,const Graph& g2 ) const {
        bool res = check_isomorphism(g1,g2);
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
    bool same_as_head() const {
        return check_isomorphism(handle.getGraph(),handle.checkout(handle.get_revision()).getGraph());
    }
    bool check() const {
        for (typename std::map<revision,Graph>::const_iterator it=snapshots.begin(); it!=snapshots.end(); ++it){
            FILE_LOG(logINFO) << "CHECK REVISION: " << it->first;
            revision r = it->first;
            archive_handle<Graph> h = ::checkout(handle,r);
            std::cout << "successfully checked out" << std::endl;
            if(!check_equality(it->second,h.getGraph())){
                FILE_LOG(logERROR) << "real graph != repository";
                return false;
            } else {
                FILE_LOG(logDEBUG1) << "CHECK REVISION: " << it->first << " SUCCESS";
            }
        }
        return true;
    }

protected:

    vertex_iterator getVertexIterator(int u){
        std::pair<vertex_iterator, vertex_iterator> vi = vertices(handle.getGraph());
        assert(std::distance(vi.first,vi.second) > u);
        vertex_iterator it_u = vi.first;
        std::advance(it_u,u);
        return it_u;
    }

    handle_type handle;
    graph_archive<Graph> archive;
    std::map<revision,Graph> snapshots;
 //   Graph current;

};




#endif // TESTS_H
