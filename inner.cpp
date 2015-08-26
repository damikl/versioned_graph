#include "versioned_graph_test.h"
#include <iostream>
#include <utility>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/topological_sort.hpp>

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
