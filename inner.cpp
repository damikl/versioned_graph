//#include <boost/config.hpp>
#include <iostream>
#include <utility>
//#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
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
    typedef typename graph_traits<Graph>::adjacency_iterator adjacency_iterator;
    typedef typename Graph::inv_adjacency_iterator inv_adjacency_iterator;
    typedef typename graph_traits<Graph>::directed_category directed_category;
    typedef typename graph_traits<Graph>::edge_parallel_category edge_parallel_category;
    typedef typename graph_traits<Graph>::vertices_size_type vertices_size_type;
    typedef typename graph_traits<Graph>::edges_size_type edges_size_type;
    typedef typename graph_traits<Graph>::degree_size_type degree_size_type;
    typedef typename Graph::out_edge_list_selector out_edge_list_selector;
    typedef typename Graph::vertex_list_selector vertex_list_selector;
    typedef typename Graph::directed_selector directed_selector;
    typedef typename Graph::edge_list_selector edge_list_selector;


    typedef typename boost::vertex_bundle_type<Graph>::type vertex_properties;
    typedef typename boost::edge_bundle_type<Graph>::type edge_properties;
    GraphTest(){
        FILE* log_fd = fopen( "init_graph_test.log", "w" );
        Output2FILE::Stream() = log_fd;
        init();
    }
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
    void check_out_edges(vertex_descriptor u,std::set<vertex_descriptor> set) const {
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
       //     ASSERT_FALSE(list.find(v)==list.end());
        }
        ASSERT_EQ(set.size(),count);
    }
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
protected:
    vertex_descriptor v1,v2,v3,v4,v5;
    Graph g;
};

class UndirectedGraphTest : public GraphTest<versioned_graph<boost::vecS, boost::vecS, boost::undirectedS,int,int>> {
//    typedef versioned_graph<boost::listS, boost::listS, boost::undirectedS,int,int> simple_graph;
public:
    void test_after_init(){
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
    }
    void test(){
        FILELog::ReportingLevel() = logDEBUG4;
        bool directed_category_res = std::is_same<boost::undirected_tag,directed_category>::value;
        ASSERT_TRUE(directed_category_res);
        typedef typename graph_traits<graph_type>::edge_parallel_category edge_parallel_category;
        bool edge_parallel_category_res = std::is_same<boost::allow_parallel_edge_tag,edge_parallel_category>::value;
        ASSERT_TRUE(edge_parallel_category_res);
        ::testing::StaticAssertTypeEq<boost::allow_parallel_edge_tag, edge_parallel_category>();

        EXPECT_NO_FATAL_FAILURE(test_after_init());

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
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(8));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v5}));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v5,{this->v1,this->v2,this->v4}));

        EXPECT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v5}));
        ASSERT_EQ(444,this->g[this->v4]);
        ASSERT_EQ(111,this->g[this->v1]);
        undo_commit(this->g);
        EXPECT_NO_FATAL_FAILURE(test_after_init());
        /*
        ASSERT_EQ(revision(2),this->g.get_current_rev());
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v2,{this->v4,this->v1,this->v3}));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_EQ(4,this->g[this->v4]);
        ASSERT_EQ(1,this->g[this->v1]);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(5));
        */

    }
};

TEST_F(UndirectedGraphTest, undirected_graph_test) {
    FILE* log_fd = fopen( "undirected_graph_test.txt", "w" );
    Output2FILE::Stream() = log_fd;
    ASSERT_NO_FATAL_FAILURE(this->test());
}

TEST(VersionedGraphTest, conceptCheck){
    FILE* log_fd = fopen( "inner_check_concepts.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;

    typedef versioned_graph<boost::vecS, boost::vecS, boost::undirectedS,int,int> simple_graph;
    simple_graph sg;

    typedef typename graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    typedef typename graph_traits<simple_graph>::edge_descriptor edge_descriptor;
    typedef typename graph_traits<simple_graph>::vertex_iterator vertex_iterator;
    typedef typename graph_traits<simple_graph>::edge_iterator edge_iterator;
    typedef typename graph_traits<simple_graph>::out_edge_iterator out_edge_iterator;
    typedef typename graph_traits<simple_graph>::adjacency_iterator adjacency_iterator;
    typedef typename simple_graph::inv_adjacency_iterator inv_adjacency_iterator;
    typedef typename graph_traits<simple_graph>::directed_category directed_category;
    ::testing::StaticAssertTypeEq<boost::undirected_tag, directed_category>();
    typedef typename graph_traits<simple_graph>::edge_parallel_category edge_parallel_category;
    ::testing::StaticAssertTypeEq<boost::allow_parallel_edge_tag, edge_parallel_category>();
    typedef typename graph_traits<simple_graph>::vertices_size_type vertices_size_type;
    typedef typename graph_traits<simple_graph>::edges_size_type edges_size_type;
    typedef typename graph_traits<simple_graph>::degree_size_type degree_size_type;
    typedef typename simple_graph::out_edge_list_selector out_edge_list_selector;
    typedef typename simple_graph::vertex_list_selector vertex_list_selector;
    typedef typename simple_graph::directed_selector directed_selector;
    typedef typename simple_graph::edge_list_selector edge_list_selector;


    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    vertex_descriptor v1 = add_vertex(1,sg);
    vertex_descriptor v2 = add_vertex(2,sg);
    vertex_descriptor v3 = add_vertex(3,sg);
    vertex_descriptor v4 = add_vertex(4,sg);
    edge_descriptor e;
    bool result = false;
    boost::tie(e,result) = add_edge(v1,v2,9,sg);
    assert(result);
    boost::tie(e,result) = add_edge(v1,v3,8,sg);
    assert(result);
    boost::tie(e,result) = add_edge(v2,v4,7,sg);
    assert(result);
    boost::tie(e,result) = add_edge(v1,v4,11,sg);
    assert(result);
    boost::tie(e,result) = add_edge(v2,v3,12,sg);
    assert(result);
    commit(sg);
    boost::remove_edge(e,sg);
    {
        std::pair<vertex_iterator, vertex_iterator> vi = vertices(sg);
        ASSERT_EQ(4,std::distance(vi.first,vi.second));
        ASSERT_EQ(4,boost::num_vertices(sg));
    }
    {
        std::pair<edge_iterator, edge_iterator> ei = boost::edges(sg);
        ASSERT_EQ(4,boost::num_edges(sg));
        ASSERT_EQ(4,std::distance(ei.first,ei.second));

        // count edges marked as removed
        ASSERT_EQ(5,boost::num_edges(sg.get_self()));
        auto priv_ei = edges(sg.get_self());
        ASSERT_EQ(5,std::distance(priv_ei.first,priv_ei.second));
    }
    {
        std::pair<out_edge_iterator, out_edge_iterator> ei = out_edges(v1,sg);
        unsigned int count = 0;
        for(out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++count;
            ASSERT_EQ(v1,boost::source(*edge_iter,sg));
        }
        ASSERT_EQ(3,count);
        ASSERT_EQ(3,out_degree(v1,sg));
        count = 0;
        ei = out_edges(v2,sg);
        for(out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++count;
            ASSERT_EQ(v2,boost::source(*edge_iter,sg));
        }
        ASSERT_EQ(2,count);
        ASSERT_EQ(2,out_degree(v2,sg));
        count = 0;
        ei = out_edges(v3,sg);
        for(out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++count;
            ASSERT_EQ(v3,boost::source(*edge_iter,sg));
        }
        ASSERT_EQ(1,count);
        ASSERT_EQ(1,out_degree(v3,sg));
        count = 0;
//        ASSERT_TRUE(ei.first==ei.second);
        ei = out_edges(v4,sg);
        for(out_edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            ++count;
            ASSERT_EQ(v4,boost::source(*edge_iter,sg));
        }
        ASSERT_EQ(2,count);
        ASSERT_EQ(2,out_degree(v4,sg));
//        ASSERT_TRUE(ei.first==ei.second);
    }
    {
        std::pair<adjacency_iterator, adjacency_iterator> ei = adjacent_vertices(v1,sg);
        unsigned int count = 0;
        for(adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v1 << ": " << v;
            ASSERT_TRUE(v==v2 || v==v3 || v==v4);
        }
        ASSERT_EQ(3,count);
        count = 0;
        ei = adjacent_vertices(v2,sg);
        for(adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v2 << ": " << v;
            ASSERT_TRUE(v==v4 || v==v1);
        }
        ASSERT_EQ(2,count);
        count = 0;
        ei = adjacent_vertices(v3,sg);
        for(adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v3 << ": " << v;
            ASSERT_TRUE(v==v1);
        }
        ASSERT_EQ(1,count);
        count = 0;
        ei = adjacent_vertices(v4,sg);
        for(adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v4 << ": " << v;
            ASSERT_TRUE(v==v1 || v==v2);
        }
        ASSERT_EQ(2,count);
    }
    {
        std::pair<inv_adjacency_iterator, inv_adjacency_iterator> ei = inv_adjacent_vertices(v1,sg);
        unsigned int count = 0;
        for(inv_adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v1 << ": " << v;
            ASSERT_TRUE(v==v2 || v==v3 || v==v4);
        }
        ASSERT_EQ(3,count);
        count = 0;
        ei = inv_adjacent_vertices(v2,sg);
        for(inv_adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v2 << ": " << v;
            ASSERT_TRUE(v==v4 || v==v1);
        }
        ASSERT_EQ(2,count);
        count = 0;
        ei = inv_adjacent_vertices(v3,sg);
        for(inv_adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v3 << ": " << v;
            ASSERT_TRUE(v==v1);
        }
        ASSERT_EQ(1,count);
        count = 0;
        ei = inv_adjacent_vertices(v4,sg);
        for(inv_adjacency_iterator iter = ei.first; iter != ei.second; ++iter) {
            ++count;
            vertex_descriptor v = *iter;
            FILE_LOG(logDEBUG1) << "adjacent vertex to "<< v4 << ": " << v;
            ASSERT_TRUE(v==v1 || v==v2);
        }
        ASSERT_EQ(2,count);
    }
}

TEST(VersionedGraphTest, SimpleExample) {
    FILE* log_fd = fopen( "inner_basic_tests.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;
    typedef versioned_graph<boost::vecS, boost::vecS, boost::directedS,int,string> simple_graph;
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
    typedef versioned_graph<boost::vecS, boost::vecS, boost::directedS> simple_graph;
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
    typedef versioned_graph<boost::vecS, boost::vecS, boost::directedS,ColorProperty> simple_graph;
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
