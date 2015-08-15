#include <iostream>
#include <utility>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include "gtest/gtest.h"

TEST(VersionedGraphTest1, normalTopologicalSort) {
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
    ASSERT_TRUE(edge(v1,v4,sg).second);
    ASSERT_EQ(5,num_edges(sg));
    vertex_descriptor v5 = add_vertex(sg);
    add_edge(v5,v4,sg);
    add_edge(v3,v5,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));

    // Perform a topological sort.
    std::vector<vertex_descriptor> topo_order;
    boost::topological_sort(sg, std::back_inserter(topo_order));
    // Print the results.
    for(std::vector<vertex_descriptor>::const_iterator i = topo_order.begin();i != topo_order.end();++i)
    {
        std::cout << *i << std::endl;
        cout << *i;
    }

}
