/***
 * author: Damian Lipka
 *
 * */


#include "versioned_graph_test.h"
#include <iostream>
#include <utility>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/topological_sort.hpp>

TEST(VersionedGraphTest, SimpleExample) {
    using namespace boost;
    using namespace std;
    typedef versioned_graph<adjacency_list<boost::vecS, boost::listS, boost::directedS,int,string,long>> simple_graph;
    simple_graph sg;
    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
    ::testing::StaticAssertTypeEq<int, vertex_properties>();
    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    ::testing::StaticAssertTypeEq<std::string, edge_properties>();
    typedef typename boost::graph_bundle_type<simple_graph>::type graph_properties;
    ::testing::StaticAssertTypeEq<long, graph_properties>();
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(1,sg);
    vertex_descriptor v2 = add_vertex(2,sg);
    vertex_descriptor v3 = add_vertex(3,sg);
    vertex_descriptor v4 = add_vertex(6,sg);
    add_edge(v1,v2,std::string("A1"),sg);
    add_edge(v1,v3,"A2",sg);
    add_edge(v2,v4,"A3",sg);
    add_edge(v1,v4,"A4",sg);
    add_edge(v2,v3,"A5",sg);
    sg[v4] = 4;
    sg[edge(v2,v4,sg).first] = "A6";
    sg[graph_bundle] = 30;
    ASSERT_EQ(4,num_vertices(sg));
    ASSERT_EQ(5,num_edges(sg));
    commit(sg);
    sg[v4] = 5;
    sg[graph_bundle] = 31;
    remove_edge(v1,v4,sg);
    ASSERT_EQ(4,num_edges(sg));
    EXPECT_FALSE(edge(v1,v4,sg).second);
    commit(sg);
    sg[v4] = 6;
    sg[graph_bundle] = 32;
    simple_graph copy(sg);
    undo_commit(sg);
    ASSERT_TRUE(edge(v1,v4,sg).second);
    ASSERT_EQ(5,num_edges(sg));
    ASSERT_EQ(4,sg[v4]);
    ASSERT_EQ(4,num_edges(copy));
    ASSERT_EQ(6,copy[vertex(3,copy)]);
    ASSERT_EQ(32,copy[graph_bundle]);
    ASSERT_EQ(30,sg[graph_bundle]);
    vertex_descriptor v5 = add_vertex(5,sg);
    add_edge(v5,v4,"13",sg);
    add_edge(v3,v5,"14",sg);
    sg[v4] = 7;
    sg[graph_bundle] = 33;
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));
    ASSERT_EQ(5,sg[v5]);
    revert_changes(sg);
    ASSERT_EQ(4,num_vertices(sg));
    ASSERT_EQ(5,num_edges(sg));
    ASSERT_EQ(4,sg[v4]);
    ASSERT_EQ(30,sg[graph_bundle]);

}

TEST(VersionedGraphTest, withoutTypes) {
    using namespace boost;
    using namespace std;
    typedef versioned_graph<adjacency_list<boost::vecS, boost::listS, boost::directedS>> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
    ::testing::StaticAssertTypeEq<boost::no_property, vertex_properties>();
    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    ::testing::StaticAssertTypeEq<boost::no_property, edge_properties>();
    typedef typename boost::graph_bundle_type<simple_graph>::type graph_properties;
    ::testing::StaticAssertTypeEq<boost::no_property, graph_properties>();


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
    commit(sg);
    remove_edge(v1,v4,sg);
    ASSERT_EQ(4,num_edges(sg));
    EXPECT_FALSE(edge(v1,v4,sg).second);
    revert_changes(sg);
    ASSERT_TRUE(edge(v1,v4,sg).second);
    ASSERT_EQ(5,num_edges(sg));
    vertex_descriptor v5 = add_vertex(sg);
    add_edge(v5,v4,sg);
    add_edge(v3,v5,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));
    revert_changes(sg);
    ASSERT_EQ(4,num_vertices(sg));
    ASSERT_EQ(5,num_edges(sg));
    auto iterv = vertices(sg);
    ASSERT_EQ(4,distance(iterv.first,iterv.second));
    auto itere = edges(sg);
    ASSERT_EQ(5,distance(itere.first,itere.second));

    typedef versioned_graph<adjacency_list<boost::multisetS, boost::listS, boost::undirectedS>> parallel_graph;
    typedef typename boost::graph_traits<parallel_graph>::vertex_descriptor vertex_descr;
    parallel_graph g;
    vertex_descr nv1 = add_vertex(g);
    vertex_descr nv2 = add_vertex(g);
    vertex_descr nv3 = add_vertex(g);
    vertex_descr nv4 = add_vertex(g);
    add_edge(nv1,nv2,g);
    add_edge(nv1,nv3,g);
    add_edge(nv2,nv4,g);
    add_edge(nv1,nv4,g);
    add_edge(nv2,nv3,g);
    add_edge(nv2,nv3,g);
    typedef typename graph_traits<parallel_graph>::edge_parallel_category category;
    ::testing::StaticAssertTypeEq<boost::allow_parallel_edge_tag, category>();
    auto p_iter = edge_range(nv2,nv3,g);
    ASSERT_EQ(2,std::distance(p_iter.first,p_iter.second));

}

TEST(VersionedGraphTest, normalTopologicalSort) {
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

//    commit(sg);
//    remove_edge(v1,v4,sg);
//    ASSERT_EQ(4,num_edges(sg));
//    EXPECT_FALSE(edge(v1,v4,sg).second);
//    undo_commit(sg);
//    cout << "made undo" << endl;
    ASSERT_TRUE(edge(v1,v4,sg).second);
    ASSERT_EQ(5,num_edges(sg));
    vertex_descriptor v5 = add_vertex(sg);
    add_edge(v5,v4,sg);
    add_edge(v3,v5,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));


    std::vector<vertex_descriptor> topo_order;
    boost::topological_sort(sg, std::back_inserter(topo_order));
    ASSERT_EQ(5,std::distance(topo_order.begin(),topo_order.end()));

}

TEST(VersionedGraphTest, checkTopologicalSort) {
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

    commit(sg);

    remove_edge(v1,v4,sg);
    ASSERT_EQ(4,num_edges(sg));
    EXPECT_FALSE(edge(v1,v4,sg).second);
    revert_changes(sg);
    ASSERT_TRUE(edge(v1,v4,sg).second);
    ASSERT_EQ(5,num_edges(sg));

    vertex_descriptor v5 = add_vertex(sg);
    add_edge(v5,v4,sg);
    add_edge(v3,v5,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));

    std::deque<int> topo_order;
    boost::topological_sort(sg.get_base_graph(), std::front_inserter(topo_order));
    ASSERT_EQ(5,std::distance(topo_order.begin(),topo_order.end()));

}
