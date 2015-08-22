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

namespace boost {

template<typename Graph>
struct property_map<Graph, vertex_index_t > {
    typedef std::map<typename graph_traits<Graph>::vertex_descriptor, int> my_vertex_mapping;
    typedef index_map< my_vertex_mapping > const_type;
    //typedef type const_type ; //-- we do not define type as "vertex_index_t" map is read-only
};

template<typename Graph>
typename property_map<Graph, vertex_index_t >::const_type get(vertex_index_t, const Graph & g )
{
    typedef std::map<typename graph_traits<Graph>::vertex_descriptor, int> vertex_mapping;
    vertex_mapping v_indexes;
    index_map< vertex_mapping > v1_index_map(v_indexes);
    int id = 0;
    typename graph_traits<Graph>::vertex_iterator i, end;
    for (boost::tie(i, end) = vertices(g); i != end; ++i, ++id) {
        FILE_LOG(logDEBUG4) << "put property: " << *i << " -> " << id;
        put(v1_index_map, *i, id);
    }
    return v1_index_map;
}


} //namespace boost

using namespace boost;
using namespace std;

template < typename Graph> void
print_dependencies(std::ostream & out, const Graph & g) {
  typename boost::graph_traits < Graph >::edge_iterator ei, ei_end;
  for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
    out << boost::source(*ei, g) << " (" << g[boost::source(*ei, g)] << ") -$>$ "
      << boost::target(*ei, g) << " (" << g[boost::target(*ei, g)] << ") property: " << g[*ei] << std::endl;
}

class InnerBacktrackingTest : public ::testing::Test {
 protected:
  typedef boost::versioned_graph<boost::vecS, boost::listS, boost::bidirectionalS,external_data,external_data> graph_type;
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
                    int tmp = boost::out_degree(*vs,graph);
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
                            FILE_LOG(logDEBUG1) << "isomorphism found";
                            break;
                        }
                        FILE_LOG(logDEBUG1) << "isomorphism not found";
                    }
                    if(!already_exist){
                        FILE_LOG(logDEBUG1) << "backtrack: clone graph";
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
                    if(level<5){
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


