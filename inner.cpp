//#include <boost/config.hpp>
#include <iostream>
#include <utility>
//#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include "utils.h"
#include <boost/iterator/filter_iterator.hpp>
#include <boost/graph/topological_sort.hpp>
#include "versioned_graph.h"
#include "subgraph_test.h"
#include "isomorphism_checking.h"
#include "gtest/gtest.h"

using namespace std;

namespace std{
    template<>
    struct hash<external_data>
    {
        typedef external_data argument_type;
        typedef size_t result_type;

        result_type operator()(argument_type const& s) const
        {
            return hash<int>()(s.value);
        }
    };
}

TEST(VersionedGraphTest, conceptCheck){
    FILE* log_fd = fopen( "inner_check_concepts.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;

    typedef versioned_graph<boost::vecS, boost::vecS, boost::directedS,int,int> simple_graph;
    simple_graph sg;
    typedef typename graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    typedef typename graph_traits<simple_graph>::edge_descriptor edge_descriptor;
    typedef typename graph_traits<simple_graph>::vertex_iterator vertex_iterator;
    typedef typename graph_traits<simple_graph>::edge_iterator edge_iterator;
    typedef typename graph_traits<simple_graph>::out_edge_iterator out_edge_iterator;
    typedef typename graph_traits<simple_graph>::adjacency_iterator adjacency_iterator;
    typedef typename simple_graph::inv_adjacency_iterator inv_adjacency_iterator;
    typedef typename graph_traits<simple_graph>::directed_category directed_category;
    typedef typename graph_traits<simple_graph>::edge_parallel_category edge_parallel_category;
    typedef typename graph_traits<simple_graph>::vertices_size_type vertices_size_type;
    typedef typename graph_traits<simple_graph>::edges_size_type edges_size_type;
    typedef typename graph_traits<simple_graph>::degree_size_type degree_size_type;
    typedef typename simple_graph::out_edge_list_selector out_edge_list_selector;
    typedef typename simple_graph::vertex_list_selector vertex_list_selector;
    typedef typename simple_graph::directed_selector directed_selector;
    typedef typename simple_graph::edge_list_selector edge_list_selector;


    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    auto v1 = add_vertex(1,sg);
    auto v2 = add_vertex(2,sg);
    auto v3 = add_vertex(3,sg);
    auto v4 = add_vertex(4,sg);
    add_edge(9,v1,v2,sg);
    add_edge(8,v1,v3,sg);
    add_edge(7,v2,v4,sg);
    add_edge(11,v1,v4,sg);
    add_edge(12,v2,v3,sg);
}

TEST(VersionedGraphTest, HandlesZeroInput) {
    FILE* log_fd = fopen( "inner_basic_tests.txt", "w" );
    Output2FILE::Stream() = log_fd;
    using namespace boost;
    using namespace std;
    typedef versioned_graph<boost::vecS, boost::vecS, boost::directedS,int,int> simple_graph;
    simple_graph sg;
//    typedef typename graph_traits<simple_graph>::vertices_size_type size_type;
//    typedef typename boost::vertex_bundle_type<simple_graph>::type vertex_properties;
//    typedef typename boost::edge_bundle_type<simple_graph>::type edge_properties;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(1,sg);
    vertex_descriptor v2 = add_vertex(2,sg);
    vertex_descriptor v3 = add_vertex(3,sg);
    vertex_descriptor v4 = add_vertex(4,sg);
    add_edge(v1,v2,9,sg);
    add_edge(v1,v3,8,sg);
    add_edge(v2,v4,7,sg);
    add_edge(v1,v4,11,sg);
    add_edge(v2,v3,12,sg);
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
    undo_commit(sg);
    FILE_LOG(logDEBUG1) << "made undo";
    ASSERT_TRUE(edge(v1,v4,sg).second);
    FILE_LOG(logDEBUG1) << "edge recreated";
    ASSERT_EQ(5,num_edges(sg));
    FILE_LOG(logDEBUG1) << "count match";
    ASSERT_EQ(4,sg[v4]);
    FILE_LOG(logDEBUG1) << "attribute match";
    vertex_descriptor v5 = add_vertex(5,sg);
    add_edge(v5,v4,13,sg);
    add_edge(v3,v5,14,sg);
    ASSERT_EQ(5,num_vertices(sg));
    ASSERT_EQ(7,num_edges(sg));
    ASSERT_EQ(5,sg[v5]);


    typedef boost::graph_traits<simple_graph>::vertex_iterator vertex_iterator;

    //Tried to make this section more clear, instead of using tie, keeping all
    //the original types so it's more clear what is going on
    std::pair<vertex_iterator, vertex_iterator> vi = vertices(sg);
    for(vertex_iterator vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
        std::cout << "(" << *vertex_iter << ")\n";
    }

    typedef boost::graph_traits<simple_graph>::edge_iterator edge_iterator;

    //Tried to make this section more clear, instead of using tie, keeping all
    //the original types so it's more clear what is going on
    std::pair<edge_iterator, edge_iterator> ei = edges(sg);
    for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        std::cout << "(" << source(*edge_iter, sg) << ", " << target(*edge_iter, sg) << ")\n";
        FILE_LOG(logDEBUG1) << "("  << source(*edge_iter, sg) << ", " << target(*edge_iter, sg) << ")";
    }

    /*
    FILE_LOG(logDEBUG1) << "make tolological sort";
    // Perform a topological sort.
    std::deque<int> topo_order;
    boost::topological_sort(sg, std::front_inserter(topo_order));
    // Print the results.
    FILE_LOG(logDEBUG1) << "Print the results";
    for(std::deque<int>::const_iterator i = topo_order.begin();i != topo_order.end();++i)
    {
        std::cout << *i << std::endl;
        FILE_LOG(logDEBUG1) << *i;
    }
    */
}

template < typename Graph> void
print_dependencies(std::ostream & out, const Graph & g) {
  typename boost::graph_traits < Graph >::edge_iterator ei, ei_end;
  for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
    out << boost::source(*ei, g) << " (" << g[boost::source(*ei, g)] << ") -$>$ "
      << boost::target(*ei, g) << " (" << g[boost::target(*ei, g)] << ") property: " << g[*ei] << std::endl;
}

class InnerBacktrackingTest : public ::testing::Test {
 protected:
  typedef boost::versioned_graph<boost::listS, boost::listS, boost::bidirectionalS,external_data,external_data> graph_type;
  typedef typename boost::graph_traits<graph_type>::vertex_iterator vertex_iterator;
    InnerBacktrackingTest() :graph() {
        FILE* log_fd = fopen( "mylogfile_inner_backtracking.txt", "w" );
        Output2FILE::Stream() = log_fd;
    }
    virtual void SetUp() {
        size = 4;
        for(unsigned int i=0; i<size;++i){
            boost::add_vertex(graph);
        }
    }
    virtual void TearDown() { }

    void backtrack(list<graph_type>& results, int target_degree, int level){
        vertex_iterator vi, vi_end;
        for(boost::tie(vi, vi_end) = boost::vertices(graph); vi!=vi_end; ++vi){
            vertex_iterator vi2, vi2_end;
            FILE_LOG(logDEBUG1) << "backtrack: jump to " << *vi << " -> ";
            for(boost::tie(vi2, vi2_end) = boost::vertices(graph); vi2!=vi2_end; ++vi2){
                FILE_LOG(logDEBUG1) << "backtrack: processing edge: " << *vi << " -> " << *vi2 << " on level " << level;
                if((*vi==*vi2) || boost::edge(*vi,*vi2,graph).second){
                    FILE_LOG(logDEBUG1) << "backtrack: same vertices or edge already exist, jump to " << *vi << " -> " << *vi2;
                    continue;
                }
                BOOST_ASSERT_MSG(size==num_vertices(graph),("Graph has " + to_string( num_vertices(graph) ) + " vertices " + " not " + to_string(size)).c_str());
                ASSERT_EQ(size,num_vertices(graph));
                commit(graph);// savepoint
                FILE_LOG(logDEBUG1) << "backtrack: try add edge: " << *vi << " -> " << *vi2;
                auto e = add_edge(*vi,*vi2,graph);
                ASSERT_TRUE(e.second);
                auto desc = e.first;
                FILE_LOG(logDEBUG1) << "backtrack: add edge: " << *vi << " -> " << *vi2;
                int& i = graph[desc].value;
                i = graph[desc].value+1; // alter attribute
                int min_degree=10000, max_degree = 0;
                typename boost::graph_traits<graph_type>::vertex_iterator vs,ve;
                for (boost::tie(vs, ve) = boost::vertices(graph); vs != ve; ++vs){
                    int tmp = out_degree(*vs,graph);
                    FILE_LOG(logDEBUG1) << "backtrack: out_degree for: " << *vs << " is " << tmp;
                    if(tmp<min_degree){
                        min_degree=tmp;
                    }
                    if(tmp>max_degree){
                        max_degree=tmp;
                    }
                }
                FILE_LOG(logDEBUG1) << "backtrack: max_degree: " << max_degree << " min_degree: " << min_degree << " num_edges: "<< num_edges(graph);
                if((max_degree==target_degree) && (min_degree ==target_degree)){
                    bool already_exist = false;
                    for(graph_type g : results){
                        FILE_LOG(logDEBUG1) << "check isomorphism";
                        if(check_isomorphism(g,graph)){
                            already_exist = true;
                            break;
                        }
                        FILE_LOG(logDEBUG1) << "isomorphism not found";
                    }
                    if(!already_exist){
                        graph_type g(graph);
                        results.push_front(g);
                        FILE_LOG(logDEBUG1) << "backtrack: success, rollback to rev: " << g.get_current_rev();
                    } else {
                        FILE_LOG(logDEBUG1) << "backtrack: already found similar graph, rollback to rev: " << graph.get_current_rev().get_rev()-1;
                    }
                    boost::undo_commit(graph);
                    return;
                }
                BOOST_ASSERT_MSG(size==num_vertices(graph),("backtrack: Graph has " + to_string( num_vertices(graph) ) + " vertices " + " not " + to_string(size)).c_str());
                ASSERT_EQ(size,num_vertices(graph));
                if(max_degree>target_degree){
                    FILE_LOG(logDEBUG1) << "backtrack: failed, rollback to rev: " << graph.get_current_rev();
                    boost::undo_commit(graph);
                    ASSERT_EQ(size,num_vertices(graph));
                    continue;
                }
                if(min_degree<target_degree){
                    FILE_LOG(logDEBUG1) << "backtrack: small min_degree: " << min_degree;
                    if(level<4){
                        backtrack(results,target_degree,level+1);
                    } else {
                        FILE_LOG(logDEBUG1) << "backtrack: level too high: " << level;
                    }
                }
            }
        }
    }

    int backtrack(){
        list<graph_type> results;
        backtrack(results,2,1);
        FILE_LOG(logDEBUG1) << "found " << results.size() << " results" << endl;
        for(graph_type g : results){
            print_dependencies(cout,g);
            cout << endl;
        }
        return results.size();
    }
    unsigned int size;
    graph_type graph;
};

TEST_F(InnerBacktrackingTest, checkFullBacktracking) {
    FILE* log_fd = fopen( "checkFullBacktracking.txt", "w" );
    Output2FILE::Stream() = log_fd;
     FILELog::ReportingLevel() = logDEBUG4;
     ASSERT_GE(this->backtrack(),1);
}

