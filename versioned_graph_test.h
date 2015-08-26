#ifndef VERSIONED_GRAPH_TEST_H
#define VERSIONED_GRAPH_TEST_H

#include "versioned_graph.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include "gtest/gtest.h"

using namespace std;
using namespace boost;

template<typename T1,typename T2>
struct assert_type_eq{
    static_assert(std::is_same<T1,T2>::value,"Types are not equal");
};

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

#endif // VERSIONED_GRAPH_TEST_H
