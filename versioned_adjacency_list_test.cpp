#include "versioned_graph_test.h"
#include <iostream>
#include <utility>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/topological_sort.hpp>


template<typename graph>
struct edge_remove_predicate{
    typedef typename graph_traits<graph>::edge_descriptor edge_descriptor;
    typedef typename boost::edge_bundle_type<graph>::type edge_bundled;
    edge_remove_predicate(const graph& g,int mod=2) : g(g), mod(mod){}
    bool operator()(edge_descriptor e){
        edge_bundled prop = g[e];
        return prop % mod==0;
    }
    const graph& g;
    int mod;
};
template<typename graph>
edge_remove_predicate<graph> make_predicate(const graph& g,int mod=2){
    edge_remove_predicate<graph> pred(g,mod);
    return pred;
}


template<typename adjacency_list_graph>
class ListGraphTest : public GraphTest<adjacency_list_graph>{
public:
    typedef adjacency_list_graph graph_type;
    typedef typename adjacency_list_graph::inv_adjacency_iterator inv_adjacency_iterator;
    typedef typename graph_traits<adjacency_list_graph>::vertex_descriptor vertex_descriptor;
    typedef typename graph_type::out_edge_list_selector out_edge_list_selector;
    typedef typename graph_type::vertex_list_selector vertex_list_selector;
    typedef typename graph_type::directed_selector directed_selector;
    typedef typename graph_type::edge_list_selector edge_list_selector;

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
};

class UndirectedGraphTest : public ListGraphTest<versioned_graph<adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,int,int,double>>> {
public:

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

    void test(){
        typedef detail::revision revision;
        FILELog::ReportingLevel() = logDEBUG4;
        ::testing::StaticAssertTypeEq<boost::undirected_tag, directed_category>();
        typedef typename graph_traits<graph_type>::edge_parallel_category edge_parallel_category;
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
        add_edge(v6,this->v2,16,this->g);
        add_edge(v6,this->v4,17,this->g);
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
        ASSERT_TRUE(boost::edge(this->v1,this->v4,this->g).second);
        ASSERT_TRUE(boost::edge(this->v1,this->v2,this->g).second);
        ASSERT_TRUE(boost::edge(this->v1,this->v5,this->g).second);

        ASSERT_TRUE(boost::edge(this->v5,this->v1,this->g).second);
        ASSERT_TRUE(boost::edge(this->v5,this->v2,this->g).second);
        ASSERT_TRUE(boost::edge(this->v5,this->v4,this->g).second);
        boost::remove_edge_if(make_predicate(this->g,3),this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(8));
        ASSERT_TRUE(boost::edge(this->v1,this->v4,this->g).second);
        ASSERT_FALSE(boost::edge(this->v1,this->v2,this->g).second);
        ASSERT_TRUE(boost::edge(this->v1,this->v5,this->g).second);

        ASSERT_TRUE(boost::edge(this->v5,this->v1,this->g).second);
        ASSERT_TRUE(boost::edge(this->v5,this->v2,this->g).second);
        ASSERT_FALSE(boost::edge(this->v5,this->v4,this->g).second);
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v5}));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v5,{this->v1,this->v2}));
        undo_commit(this->g);
        EXPECT_NO_FATAL_FAILURE(this->test_after_init());
        undo_commit(this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v2,{this->v4,this->v1,this->v3}));
        undo_commit(this->g);// undo of init commit change nothing
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(5));
    }
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
        typedef detail::revision revision;
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

class DirectedGraphTest : public ListGraphTest<versioned_graph<adjacency_list<boost::vecS, boost::vecS, boost::directedS,int,int,double>>> {
public:
    typedef GraphTest<versioned_graph<adjacency_list<boost::vecS, boost::vecS, boost::directedS,int,int,double>>> base_type;

    virtual void test_after_init(){
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v2,{this->v4}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v3,{}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v4,{}));

        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v2,{this->v4}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v3,{}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v4,{}));

        ASSERT_EQ(4,this->g[this->v4]);
        ASSERT_EQ(1,this->g[this->v1]);
        ASSERT_EQ(-1.1,this->g[boost::graph_bundle]);
    }

    void test(){
        FILELog::ReportingLevel() = logDEBUG4;
        typedef detail::revision revision;
        typedef typename graph_traits<graph_type>::directed_category directed_category;
        assert_type_eq<boost::directed_tag,directed_category>();
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
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(7));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(8));
        this->g[this->v4] = 444;
        this->g[this->v1] = 111;
        ASSERT_EQ(revision(4),this->g.get_current_rev());
        FILE_LOG(logDEBUG2) << "Clear vertex: " << this->v3;
        boost::clear_vertex(this->v3,this->g);
        boost::remove_vertex(this->v3,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(7));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(8));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v3,this->v5}));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v5,{this->v4}));

        EXPECT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v3,this->v5}));
        ASSERT_EQ(444,this->g[this->v4]);
        ASSERT_EQ(111,this->g[this->v1]);
        vertex_descriptor v6 = boost::add_vertex(this->g);
        add_edge(v6,this->v2,this->g);
        add_edge(v6,this->v4,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(9));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(10));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(v6,{this->v4,this->v2}));
        FILE_LOG(logDEBUG2) << "Clear vertex: " << v6;
        boost::clear_vertex(v6,this->g);
        FILE_LOG(logDEBUG2) << "Remove vertex: " << v6;
        boost::remove_vertex(v6,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(4));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(7));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(8));
        undo_commit(this->g);
        EXPECT_NO_FATAL_FAILURE(this->test_after_init());

    }
};

TEST_F(DirectedGraphTest, directed_graph_test) {
    FILE* log_fd = fopen( "directed_graph_test.txt", "w" );
    Output2FILE::Stream() = log_fd;
    ASSERT_NO_FATAL_FAILURE(this->init());
    ASSERT_NO_FATAL_FAILURE(this->test());
}