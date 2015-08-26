//#include <boost/config.hpp>
#include <iostream>
#include <utility>
//#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_utility.hpp>
//#include "utils.h"
#include <boost/iterator/filter_iterator.hpp>
#include <boost/graph/topological_sort.hpp>
#include "versioned_graph.h"
//#include "subgraph_test.h"
//#include "isomorphism_checking.h"
#include "gtest/gtest.h"
#include "boost/iostreams/stream.hpp"
#include "boost/iostreams/device/null.hpp"

using namespace std;
using namespace boost;

template<typename Graph>
class GraphTest : public testing::Test {

public:
    typedef Graph graph_type;
    typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;
    typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;
    typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;
    typedef typename graph_traits<Graph>::edge_iterator edge_iterator;
    typedef typename graph_traits<Graph>::out_edge_iterator out_edge_iterator;
    typedef typename graph_traits<Graph>::in_edge_iterator in_edge_iterator;
    typedef typename graph_traits<Graph>::adjacency_iterator adjacency_iterator;
    typedef typename graph_traits<Graph>::directed_category directed_category;
    typedef typename graph_traits<Graph>::edge_parallel_category edge_parallel_category;
    typedef typename graph_traits<Graph>::vertices_size_type vertices_size_type;
    typedef typename graph_traits<Graph>::edges_size_type edges_size_type;
    typedef typename graph_traits<Graph>::degree_size_type degree_size_type;
//    typedef typename Graph::out_edge_list_selector out_edge_list_selector;
//    typedef typename Graph::vertex_list_selector vertex_list_selector;
//    typedef typename Graph::directed_selector directed_selector;
//    typedef typename Graph::edge_list_selector edge_list_selector;


    typedef typename boost::vertex_bundle_type<Graph>::type vertex_properties;
    typedef typename boost::edge_bundle_type<Graph>::type edge_properties;
    GraphTest() : g(){}
    GraphTest(vertices_size_type n) : g(n) {}
    virtual void init(){
        v1 = add_vertex(1,g);
        v2 = add_vertex(2,g);
        v3 = add_vertex(3,g);
        v4 = add_vertex(4,g);
        edge_descriptor e;
        bool result = false;
        boost::tie(e,result) = add_edge(v1,v2,9,g);
        assert(result);
        boost::tie(e,result) = add_edge(v1,v3,8,g);
        assert(result);
        boost::tie(e,result) = add_edge(v2,v4,7,g);
        assert(result);
        boost::tie(e,result) = add_edge(v1,v4,11,g);
        assert(result);
        boost::tie(e,result) = add_edge(v2,v3,12,g);
        assert(result);
        g[boost::graph_bundle] = -1.1;
        commit(g);
        remove_edge(e,g);
        commit(g);
    }
    void check_vertices_count(unsigned int count) const {
        std::pair<vertex_iterator, vertex_iterator> vi = vertices(g);
        ASSERT_EQ(count,std::distance(vi.first,vi.second));
        ASSERT_EQ(count,boost::num_vertices(g));
    }

    void check_all_vertices_count(unsigned int count) const {
        std::pair<vertex_iterator, vertex_iterator> vi = vertices(g.get_self());
        ASSERT_EQ(count,std::distance(vi.first,vi.second));
        ASSERT_EQ(count,boost::num_vertices(g.get_self()));
    }

    void check_edges_count(unsigned int count) const{
        std::pair<edge_iterator, edge_iterator> ei = boost::edges(g);
        ASSERT_EQ(count,boost::num_edges(g));
        ASSERT_EQ(count,std::distance(ei.first,ei.second));
    }

    void check_all_edges_count(unsigned int count) const{
        std::pair<edge_iterator, edge_iterator> ei = boost::edges(g.get_self());
        ASSERT_EQ(count,boost::num_edges(g.get_self()));
        ASSERT_EQ(count,std::distance(ei.first,ei.second));
    }
    FRIEND_TEST(GraphTest, simple);

    template<typename T>
    ::testing::AssertionResult result_allowed(std::set<T> set,const T& value) const {

        if(set.end()!=set.find(value)){
            return ::testing::AssertionSuccess();
        } else {
            auto res = ::testing::AssertionFailure() << value << " is not expected, expected one of: (";
            for(typename std::set<T>::iterator it = set.begin();it!=set.end();++it){
                res << *it << " ";
            }
            res << ")";
            return res;
        }
    }
    virtual void check_out_edges(vertex_descriptor u,std::set<vertex_descriptor> set) const {
        std::pair<out_edge_iterator, out_edge_iterator> ei = out_edges(u,g);
        unsigned int out_edges_count = 0;
        FILE_LOG(logDEBUG2) << "validate out_edges for: " << u;
        for(out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++out_edges_count;
            vertex_descriptor v = boost::target(*edge_iter,g);
            FILE_LOG(logDEBUG3) << "found out_edge: "<< u << "->" << v;
            ASSERT_EQ(u,boost::source(*edge_iter,g));
            ASSERT_TRUE(result_allowed(set,v));
        }
        FILE_LOG(logDEBUG2) << "validate count of out_edges for: " << u;
        ASSERT_EQ(set.size(),out_edges_count);
        ASSERT_EQ(set.size(),out_degree(u,g));
    }

    void check_adjacency(vertex_descriptor u,std::set<vertex_descriptor> set) const {
        std::pair<adjacency_iterator, adjacency_iterator> ei = adjacent_vertices(u,g);
        unsigned int count = 0;
        for(adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< u << ": " << v;
            ASSERT_TRUE(result_allowed(set,v));
        }
        ASSERT_EQ(set.size(),count);
    }

    virtual void test_after_init(){
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v2,{this->v4,this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v4,{this->v1,this->v2}));

        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v2,{this->v4,this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v4,{this->v1,this->v2}));

        ASSERT_EQ(4,this->g[this->v4]);
        ASSERT_EQ(1,this->g[this->v1]);
        ASSERT_EQ(-1.1,this->g[boost::graph_bundle]);
    }
protected:
    vertex_descriptor v1,v2,v3,v4,v5;
    Graph g;
};

template<typename adjacency_list_graph>
class ListGraphTest : public GraphTest<adjacency_list_graph>{
public:
    typedef adjacency_list_graph graph_type;
    typedef typename adjacency_list_graph::inv_adjacency_iterator inv_adjacency_iterator;
    typedef typename graph_traits<adjacency_list_graph>::vertex_descriptor vertex_descriptor;
    void check_inv_adjacency(vertex_descriptor u,std::set<vertex_descriptor> set){
        std::pair<inv_adjacency_iterator, inv_adjacency_iterator> ei = inv_adjacent_vertices(u,this->g);
        unsigned int count = 0;
        for(inv_adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< this->v1 << ": " << v;
            ASSERT_TRUE(this->result_allowed(set,v));
        }
        ASSERT_EQ(set.size(),count);
    }

    virtual void test_after_init(){
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v2,{this->v4,this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v4,{this->v1,this->v2}));

        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v2,{this->v4,this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v4,{this->v1,this->v2}));

        ASSERT_NO_FATAL_FAILURE(this->check_inv_adjacency(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_inv_adjacency(this->v2,{this->v4,this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_inv_adjacency(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_inv_adjacency(this->v4,{this->v1,this->v2}));

        ASSERT_EQ(4,this->g[this->v4]);
        ASSERT_EQ(1,this->g[this->v1]);
        ASSERT_EQ(-1.1,this->g[boost::graph_bundle]);
    }
};

class UndirectedGraphTest : public ListGraphTest<versioned_graph<adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,int,int,double>>> {
public:

    void test(){
        FILELog::ReportingLevel() = logDEBUG4;
   //     bool directed_category_res = std::is_same<boost::undirected_tag,directed_category>::value;
        ::testing::StaticAssertTypeEq<boost::undirected_tag, directed_category>();
  //      ASSERT_TRUE(directed_category_res);
        typedef typename graph_traits<graph_type>::edge_parallel_category edge_parallel_category;
  //      bool edge_parallel_category_res = std::is_same<boost::allow_parallel_edge_tag,edge_parallel_category>::value;
//        ASSERT_TRUE(edge_parallel_category_res);
        ::testing::StaticAssertTypeEq<boost::allow_parallel_edge_tag, edge_parallel_category>();

        EXPECT_NO_FATAL_FAILURE(this->test_after_init());

        this->g[this->v4] = 44;
        this->g[this->v1] = 11;
        this->g[boost::graph_bundle] = 5.1;

        edge_descriptor e;
        bool result = false;
        this->v5 = add_vertex(5,this->g);
        boost::tie(e,result) = add_edge(this->v2,this->v5,13,this->g);
        assert(result);
        boost::tie(e,result) = add_edge(this->v1,this->v5,14,this->g);
        assert(result);
        boost::tie(e,result) = add_edge(this->v5,this->v4,15,this->g);
        assert(result);
        ASSERT_EQ(revision(3),this->g.get_current_rev());
        commit(g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        this->g[this->v4] = 444;
        this->g[this->v1] = 111;
        this->g[boost::graph_bundle] = 7.2;
        ASSERT_EQ(revision(4),this->g.get_current_rev());
        boost::clear_vertex(this->v3,this->g);
        boost::remove_vertex(this->v3,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(8));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v5}));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v5,{this->v1,this->v2,this->v4}));

        EXPECT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v5}));
        ASSERT_EQ(444,this->g[this->v4]);
        ASSERT_EQ(111,this->g[this->v1]);
        ASSERT_EQ(7.2,this->g[boost::graph_bundle]);
        vertex_descriptor v6 = boost::add_vertex(this->g);
        add_edge(v6,this->v2,this->g);
        add_edge(v6,this->v4,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(8));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(10));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(v6,{this->v4,this->v2}));
        boost::clear_vertex(v6,this->g);
        boost::remove_vertex(v6,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(8));
        undo_commit(this->g);
        EXPECT_NO_FATAL_FAILURE(this->test_after_init());
    }
};
template<typename T1,typename T2>
struct assert_type_eq{
    static_assert(std::is_same<T1,T2>::value,"Types are not equal");
};

TEST_F(UndirectedGraphTest, undirected_graph_test) {
    FILE* log_fd = fopen( "undirected_graph_test.txt", "w" );
    Output2FILE::Stream() = log_fd;
    ASSERT_NO_FATAL_FAILURE(this->init());
    ASSERT_NO_FATAL_FAILURE(this->test());
}

class BidirectionalGraphTest : public ListGraphTest<versioned_graph<adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,int,int,double>>> {
public:
    typedef GraphTest<versioned_graph<adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,int,int,double>>> base_type;
    /*
    typedef typename graph_type::inv_adjacency_iterator inv_adjacency_iterator;
    void check_inv_adjacency(vertex_descriptor u,std::set<vertex_descriptor> set){
        std::pair<inv_adjacency_iterator, inv_adjacency_iterator> ei = inv_adjacent_vertices(u,g);
        unsigned int count = 0;
        for(inv_adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v1 << ": " << v;
            ASSERT_TRUE(result_allowed(set,v));
        }
        ASSERT_EQ(set.size(),count);
    }
    */
    virtual void check_in_edges(vertex_descriptor v,std::set<vertex_descriptor> set) const {
        std::pair<in_edge_iterator, in_edge_iterator> ei = in_edges(v,g);
        unsigned int in_edges_count = 0;
        FILE_LOG(logDEBUG2) << "validate in_edges for: " << v;
        for(in_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++in_edges_count;
            vertex_descriptor u = boost::source(*edge_iter,g);
            FILE_LOG(logDEBUG3) << "found in_edge: "<< u << "->" << v;
            ASSERT_EQ(v,boost::target(*edge_iter,g));
            ASSERT_TRUE(result_allowed(set,u));
        }
        FILE_LOG(logDEBUG2) << "validate count of out_edges for: " << v;
        ASSERT_EQ(set.size(),in_edges_count);
        ASSERT_EQ(set.size(),in_degree(v,g));
    }

    virtual void test_after_init(){
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v2,{this->v4}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v3,{}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v4,{}));

        ASSERT_EQ(0,boost::in_degree(this->v1,this->g.get_self()));
        ASSERT_EQ(1,boost::in_degree(this->v2,this->g.get_self()));
        ASSERT_EQ(2,boost::in_degree(this->v3,this->g.get_self()));
        ASSERT_EQ(2,boost::in_degree(this->v4,this->g.get_self()));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v1,{}));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v2,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v4,{this->v1,this->v2}));

        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v2,{this->v4}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v3,{}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v4,{}));

        ASSERT_NO_FATAL_FAILURE(this->check_inv_adjacency(this->v1,{}));
        ASSERT_NO_FATAL_FAILURE(this->check_inv_adjacency(this->v2,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_inv_adjacency(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_inv_adjacency(this->v4,{this->v1,this->v2}));

        ASSERT_EQ(4,this->g[this->v4]);
        ASSERT_EQ(1,this->g[this->v1]);
        ASSERT_EQ(-1.1,this->g[boost::graph_bundle]);
    }

    void test(){
        FILELog::ReportingLevel() = logDEBUG4;
        typedef typename graph_traits<graph_type>::directed_category directed_category;
        assert_type_eq<boost::bidirectional_tag,directed_category>();
        typedef typename graph_traits<graph_type>::edge_parallel_category edge_parallel_category;
        ::testing::StaticAssertTypeEq<boost::allow_parallel_edge_tag, edge_parallel_category>();

        ASSERT_NO_FATAL_FAILURE(this->test_after_init());

        this->g[this->v4] = 44;
        this->g[this->v1] = 11;

        edge_descriptor e;
        bool result = false;
        this->v5 = add_vertex(5,this->g);
        boost::tie(e,result) = add_edge(this->v2,this->v5,13,this->g);
        assert(result);
        boost::tie(e,result) = add_edge(this->v1,this->v5,14,this->g);
        assert(result);
        boost::tie(e,result) = add_edge(this->v5,this->v4,15,this->g);
        assert(result);
        ASSERT_EQ(revision(3),this->g.get_current_rev());
        commit(g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        this->g[this->v4] = 444;
        this->g[this->v1] = 111;
        ASSERT_EQ(revision(4),this->g.get_current_rev());
        boost::clear_vertex(this->v3,this->g);
        boost::remove_vertex(this->v3,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(8));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v5}));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v5,{this->v4}));

        EXPECT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v5}));
        ASSERT_EQ(444,this->g[this->v4]);
        ASSERT_EQ(111,this->g[this->v1]);
        vertex_descriptor v6 = boost::add_vertex(this->g);
        add_edge(v6,this->v2,this->g);
        add_edge(v6,this->v4,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(8));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(10));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(v6,{this->v4,this->v2}));
        boost::clear_vertex(v6,this->g);
        boost::remove_vertex(v6,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(8));
        undo_commit(this->g);
        EXPECT_NO_FATAL_FAILURE(this->test_after_init());

    }
};

TEST_F(BidirectionalGraphTest, bidirectional_graph_test) {
    FILE* log_fd = fopen( "bidirectional_graph_test.txt", "w" );
    Output2FILE::Stream() = log_fd;
    ASSERT_NO_FATAL_FAILURE(this->init());
    ASSERT_NO_FATAL_FAILURE(this->test());
}

class DirectedMatrixGraphTest : public GraphTest<versioned_graph<adjacency_matrix<boost::directedS,int,int,double>>> {
public:
    typedef GraphTest<versioned_graph<adjacency_matrix<boost::directedS,int,int,double>>> base_type;
    DirectedMatrixGraphTest() : base_type(5) {}
    virtual void init(){
        enum { A, B, C, D, E, F, N };
        v1 = A;
        this->g[A]=1;
        v2 = B;
        this->g[B]=2;
        v3 = C;
        this->g[C]=3;
        v4 = D;
        this->g[D]=4;
        v5 = E;
        this->g[E]=5;
        edge_descriptor e;
        bool result = false;
        boost::tie(e,result) = add_edge(v1,v2,9,g);
        assert(result);
        boost::tie(e,result) = add_edge(v1,v3,8,g);
        assert(result);
        boost::tie(e,result) = add_edge(v2,v4,7,g);
        assert(result);
        boost::tie(e,result) = add_edge(v1,v4,11,g);
        assert(result);
        boost::tie(e,result) = add_edge(v5,v4,11,g);
        assert(result);
        boost::tie(e,result) = add_edge(v2,v3,12,g);
        assert(result);
        g[boost::graph_bundle] = -1.1;
        commit(g);
        remove_edge(e,g);
        commit(g);
    }
    virtual void check_in_edges(vertex_descriptor v,std::set<vertex_descriptor> set) const {
        std::pair<in_edge_iterator, in_edge_iterator> ei = in_edges(v,g);
        unsigned int in_edges_count = 0;
        FILE_LOG(logDEBUG2) << "validate in_edges for: " << v;
        for(in_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++in_edges_count;
            vertex_descriptor u = boost::source(*edge_iter,g);
            FILE_LOG(logDEBUG3) << "found in_edge: "<< u << "->" << v;
            ASSERT_EQ(v,boost::target(*edge_iter,g));
            ASSERT_TRUE(result_allowed(set,u));
        }
        FILE_LOG(logDEBUG2) << "validate count of out_edges for: " << v;
        ASSERT_EQ(set.size(),in_edges_count);
        ASSERT_EQ(set.size(),in_degree(v,g));
    }

    virtual void test_after_init(){
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v2,{this->v4}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v3,{}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v4,{}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v5,{this->v4}));

        ASSERT_EQ(0,boost::in_degree(this->v1,this->g.get_self()));
        ASSERT_EQ(1,boost::in_degree(this->v2,this->g.get_self()));
        ASSERT_EQ(2,boost::in_degree(this->v3,this->g.get_self()));
        ASSERT_EQ(3,boost::in_degree(this->v4,this->g.get_self()));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v1,{}));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v2,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v4,{this->v1,this->v2,this->v5}));

        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v2,{this->v4}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v3,{}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v4,{}));

        ASSERT_EQ(4,this->g[this->v4]);
        ASSERT_EQ(1,this->g[this->v1]);
        ASSERT_EQ(-1.1,this->g[boost::graph_bundle]);
    }

    void test(){
        enum { A, B, C, D, E, F, N };
        FILELog::ReportingLevel() = logDEBUG4;
        typedef typename graph_traits<graph_type>::directed_category directed_category;
        assert_type_eq<boost::bidirectional_tag,directed_category>();
        typedef typename graph_traits<graph_type>::edge_parallel_category edge_parallel_category;
        ::testing::StaticAssertTypeEq<boost::disallow_parallel_edge_tag, edge_parallel_category>();

        ASSERT_NO_FATAL_FAILURE(this->test_after_init());

        this->g[this->v4] = 44;
        this->g[this->v1] = 11;

        edge_descriptor e;
        bool result = false;
        boost::tie(e,result) = add_edge(this->v2,this->v5,13,this->g);
        assert(result);
        boost::tie(e,result) = add_edge(this->v1,this->v5,14,this->g);
        assert(result);
        boost::tie(e,result) = add_edge(this->v5,this->v2,15,this->g);
        assert(result);
        ASSERT_EQ(revision(3),this->g.get_current_rev());
        commit(g);
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(8));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(9));
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        this->g[this->v4] = 444;
        this->g[this->v1] = 111;
        ASSERT_EQ(revision(4),this->g.get_current_rev());
        boost::clear_vertex(this->v3,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(7));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(9));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v5}));

        EXPECT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v5}));
        ASSERT_EQ(444,this->g[this->v4]);
        ASSERT_EQ(111,this->g[this->v1]);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(7));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(9));
        boost::clear_vertex(this->v5,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(3));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(9));
        undo_commit(this->g);
        EXPECT_NO_FATAL_FAILURE(this->test_after_init());

    }
};

TEST_F(DirectedMatrixGraphTest, directed_graph_test) {
    FILE* log_fd = fopen( "directed_matrix_graph_test.txt", "w" );
    Output2FILE::Stream() = log_fd;
    ASSERT_NO_FATAL_FAILURE(this->init());
    ASSERT_NO_FATAL_FAILURE(this->test());
}


TEST(VersionedGraphTest, SimpleExample) {
    FILE* log_fd = fopen( "inner_basic_tests.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;
    typedef versioned_graph<adjacency_list<boost::vecS, boost::vecS, boost::directedS,int,string>> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
//    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
//    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(1,sg);
    vertex_descriptor v2 = add_vertex(2,sg);
    vertex_descriptor v3 = add_vertex(3,sg);
    vertex_descriptor v4 = add_vertex(6,sg);
    add_edge(v1,v2,std::string("9"),sg);
    add_edge(v1,v3,"8",sg);
    add_edge(v2,v4,"7",sg);
    add_edge(v1,v4,"11",sg);
    add_edge(v2,v3,"12",sg);
    sg[v4] = 4;
    ASSERT_EQ(4,num_vertices(sg));
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "start commit";
    commit(sg);
    FILE_LOG(logDEBUG1) << "commit end";
    sg[v4] = 5;
    remove_edge(v1,v4,sg);
    ASSERT_EQ(4,num_edges(sg));
    EXPECT_FALSE(edge(v1,v4,sg).second);
    commit(sg);
    sg[v4] = 6;
    undo_commit(sg);
    FILE_LOG(logDEBUG1) << "made undo";
    ASSERT_TRUE(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "count match";
    ASSERT_EQ(4,sg[v4]);
    FILE_LOG(logDEBUG1) << "attribute match";
    vertex_descriptor v5 = add_vertex(5,sg);
    add_edge(v5,v4,"13",sg);
    add_edge(v3,v5,"14",sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));
    ASSERT_EQ(5,sg[v5]);
}

TEST(VersionedGraphTest, withoutTypes) {
    FILE* log_fd = fopen( "checkWithoutTypes.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;
    typedef versioned_graph<adjacency_list<boost::vecS, boost::vecS, boost::directedS>> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
//    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
//    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(sg);
    vertex_descriptor v2 = add_vertex(sg);
    vertex_descriptor v3 = add_vertex(sg);
    vertex_descriptor v4 = add_vertex(sg);
    add_edge(v1,v2,sg);
    add_edge(v1,v3,sg);
    add_edge(v2,v4,sg);
    add_edge(v1,v4,sg);
    add_edge(v2,v3,sg);
    FILE_LOG(logDEBUG1) << "start commit";
    commit(sg);
    FILE_LOG(logDEBUG1) << "commit end";
    remove_edge(v1,v4,sg);
    ASSERT_EQ(4,num_edges(sg));
    EXPECT_FALSE(edge(v1,v4,sg).second);
    revert_changes(sg);
    FILE_LOG(logDEBUG1) << "made undo";
    ASSERT_TRUE(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "count match";
    vertex_descriptor v5 = add_vertex(sg);
    add_edge(v5,v4,sg);
    add_edge(v3,v5,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));
}

TEST(VersionedGraphTest, normalTopologicalSort) {
    FILE* log_fd = fopen( "normalTopologicalSort.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;
    typedef property<vertex_color_t, int> ColorProperty;
    typedef adjacency_list<boost::vecS, boost::vecS, boost::directedS,ColorProperty> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
//    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
//    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(sg);
    vertex_descriptor v2 = add_vertex(sg);
    vertex_descriptor v3 = add_vertex(sg);
    vertex_descriptor v4 = add_vertex(sg);
    add_edge(v1,v2,sg);
    add_edge(v1,v3,sg);
    add_edge(v2,v4,sg);
    add_edge(v1,v4,sg);
    add_edge(v2,v3,sg);
    FILE_LOG(logDEBUG1) << "start commit";
//    commit(sg);
//    FILE_LOG(logDEBUG1) << "commit end";
//    remove_edge(v1,v4,sg);
//    ASSERT_EQ(4,num_edges(sg));
//    EXPECT_FALSE(edge(v1,v4,sg).second);
//    undo_commit(sg);
//    FILE_LOG(logDEBUG1) << "made undo";
    ASSERT_TRUE(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "count match";
    vertex_descriptor v5 = add_vertex(sg);
    add_edge(v5,v4,sg);
    add_edge(v3,v5,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));


    FILE_LOG(logDEBUG1) << "make tolological sort";
    // Perform a topological sort.
    std::vector<vertex_descriptor> topo_order;
    boost::topological_sort(sg, std::back_inserter(topo_order));
    // Print the results.
    FILE_LOG(logDEBUG1) << "Print the results";
    for(std::vector<vertex_descriptor>::const_iterator i = topo_order.begin();i != topo_order.end();++i)
    {
        std::cout << *i << std::endl;
        FILE_LOG(logDEBUG1) << *i;
    }

}

TEST(VersionedGraphTest, checkTopologicalSort) {
    FILE* log_fd = fopen( "checkTopologicalSort.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;
    typedef property<vertex_color_t, int> ColorProperty;
    typedef versioned_graph<adjacency_list<boost::vecS, boost::vecS, boost::directedS,ColorProperty>> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
//    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
//    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(sg);
    vertex_descriptor v2 = add_vertex(sg);
    vertex_descriptor v3 = add_vertex(sg);
    vertex_descriptor v4 = add_vertex(sg);
    add_edge(v1,v2,sg);
    add_edge(v1,v3,sg);
    add_edge(v2,v4,sg);
    add_edge(v1,v4,sg);
    add_edge(v2,v3,sg);
    FILE_LOG(logDEBUG1) << "start commit";
    commit(sg);
    FILE_LOG(logDEBUG1) << "commit end";
    remove_edge(v1,v4,sg);
    ASSERT_EQ(4,num_edges(sg));
    EXPECT_FALSE(edge(v1,v4,sg).second);
    revert_changes(sg);
    FILE_LOG(logDEBUG1) << "made undo";
    ASSERT_TRUE(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "count match";
    vertex_descriptor v5 = add_vertex(sg);
    add_edge(v5,v4,sg);
    add_edge(v3,v5,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));


    FILE_LOG(logDEBUG1) << "make tolological sort";
    // Perform a topological sort.
    std::deque<int> topo_order;
    boost::topological_sort(sg.get_self(), std::front_inserter(topo_order));
    // Print the results.
    FILE_LOG(logDEBUG1) << "Print the results";
    for(std::deque<int>::const_iterator i = topo_order.begin();i != topo_order.end();++i)
    {
        std::cout << *i << std::endl;
        FILE_LOG(logDEBUG1) << *i;
    }

}
