#include "versioned_graph_test.h"
#include <iostream>
#include <utility>
#include <boost/graph/graph_utility.hpp>

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
        typedef detail::revision revision;
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

class UndirectedMatrixGraphTest : public GraphTest<versioned_graph<adjacency_matrix<boost::undirectedS,int,int,double>>> {
public:
    typedef GraphTest<versioned_graph<adjacency_matrix<boost::undirectedS,int,int,double>>> base_type;
    UndirectedMatrixGraphTest() : base_type(5) {}

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
    }

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

    virtual void test_after_init(){
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v2,{this->v4,this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v4,{this->v1,this->v2,this->v5}));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v5,{this->v4}));

        ASSERT_EQ(3,boost::in_degree(this->v1,this->g.get_self()));
        ASSERT_EQ(3,boost::in_degree(this->v2,this->g.get_self()));
        ASSERT_EQ(2,boost::in_degree(this->v3,this->g.get_self()));
        ASSERT_EQ(3,boost::in_degree(this->v4,this->g.get_self()));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v2,{this->v4,this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v4,{this->v1,this->v2,this->v5}));
        ASSERT_NO_FATAL_FAILURE(this->check_in_edges(this->v5,{this->v4}));


        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v3}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v2,{this->v4,this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v3,{this->v1}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v4,{this->v1,this->v2,this->v5}));
        ASSERT_NO_FATAL_FAILURE(this->check_adjacency(this->v5,{this->v4}));

        ASSERT_EQ(4,this->g[this->v4]);
        ASSERT_EQ(1,this->g[this->v1]);
        ASSERT_EQ(-1.1,this->g[boost::graph_bundle]);
    }

    void test(){
        typedef detail::revision revision;
        enum { A, B, C, D, E, F, N };
        FILELog::ReportingLevel() = logDEBUG4;
        typedef typename graph_traits<graph_type>::directed_category directed_category;
        assert_type_eq<boost::undirected_tag,directed_category>();
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
        boost::tie(e,result) = add_edge(this->v5,this->v3,15,this->g);
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
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(9));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v1,{this->v4,this->v2,this->v5}));
        EXPECT_NO_FATAL_FAILURE(this->check_out_edges(this->v5,{this->v4,this->v1,this->v2}));

        EXPECT_NO_FATAL_FAILURE(this->check_adjacency(this->v1,{this->v4,this->v2,this->v5}));
        EXPECT_NO_FATAL_FAILURE(this->check_adjacency(this->v5,{this->v4,this->v1,this->v2}));
        ASSERT_EQ(444,this->g[this->v4]);
        ASSERT_EQ(111,this->g[this->v1]);
        boost::clear_vertex(this->v5,this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_all_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(3));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(9));
        undo_commit(this->g);
        EXPECT_NO_FATAL_FAILURE(this->test_after_init());
        undo_commit(this->g);
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_out_edges(this->v2,{this->v4,this->v1,this->v3}));
        undo_commit(this->g);// undo of init commit change nothing
        ASSERT_NO_FATAL_FAILURE(this->check_vertices_count(5));
        ASSERT_NO_FATAL_FAILURE(this->check_edges_count(6));
        ASSERT_NO_FATAL_FAILURE(this->check_all_edges_count(6));
    }
};

TEST_F(UndirectedMatrixGraphTest, undirected_graph_test) {
    FILE* log_fd = fopen( "undirected_matrix_graph_test.txt", "w" );
    Output2FILE::Stream() = log_fd;
    ASSERT_NO_FATAL_FAILURE(this->init());
    ASSERT_NO_FATAL_FAILURE(this->test());
}

