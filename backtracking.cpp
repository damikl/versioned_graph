#include "gtest/gtest.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
#include "archive.h"
#include "handler.h"
#include "subgraph_test.h"

using namespace boost;
using namespace std;

template<typename small_graph, typename large_graph>
struct attribute_equals{
    attribute_equals(const small_graph& s,const large_graph& l) : sgraph(s),lgraph(l){

    }
    template<typename descriptor1, typename descriptor2>
    bool operator()(descriptor1 desc1, descriptor2 desc2)const{
        return sgraph[desc1] == lgraph[desc2];
    }
    const small_graph& sgraph;
    const large_graph& lgraph;
};

template <typename small_graph, typename large_graph>
attribute_equals<small_graph,large_graph> make_attribute_equals_equivalent(const small_graph &g1,const large_graph& g2) {
    return attribute_equals<small_graph,large_graph>(g1,g2);
}


template <typename Graph1, typename Graph2>
struct expand_graph_callback {
    typedef typename boost::graph_traits<Graph1>::vertex_descriptor graph1_vertex;
    typedef typename boost::graph_traits<Graph2>::vertex_descriptor graph2_vertex;
    typedef typename boost::graph_traits<Graph1>::edge_descriptor graph1_edge;
    typedef typename boost::graph_traits<Graph2>::edge_descriptor graph2_edge;
    expand_graph_callback(const Graph1 &graph1, const Graph2 &graph2) : graph1_(graph1), graph2_(graph2) {
    }
    template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
    bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1 ) {
        std::cout << "---START-------------" << std::endl;
        typename boost::graph_traits<Graph1>::vertex_iterator vi, vi_end;
        for (boost::tie(vi, vi_end) = boost::vertices(graph1_); vi != vi_end; ++vi){
            graph1_vertex v = *vi;
            graph2_vertex w = boost::get(f, v);
            if(w!=boost::graph_traits<Graph2>::null_vertex()){
                graph2_[w]=graph1_[v];
                auto p2 = boost::edge(w,v,graph2_);
                auto p1 = boost::edge(v,w,graph1_);
                if(p2.second){
                    graph2_edge e2 = p2.first;
                    graph1_edge e1 = p1.first;
                    graph2_[e2]=graph1_[e1];
                }
            }
        }
        std::cout << "---END-------------" <<  std::endl;

        return true;
    }
  private:
    const Graph1 &graph1_;
    const Graph2 &graph2_;

};

template<typename pattern_graph_type,
         typename result_graph_type,
         typename edges_predicate,
         typename vertex_predicate,
         typename sub_graph_iso_map_callback>
class rule{
public:
    rule(const pattern_graph_type& pattern,
         const result_graph_type& result,
         const edges_predicate& e_pred,
         const vertex_predicate& v_pred,
         const sub_graph_iso_map_callback& c) :
                pattern_graph(pattern),
                result_graph(result),
                edges_pred(e_pred),
                vertex_pred(v_pred),
                callback(c) {}
    const pattern_graph_type & get_pattern_graph()const{
        return pattern_graph;
    }
    const result_graph_type & get_result_graph() const{
        return result_graph;
    }
    template<typename graph>
    bool execute(graph& g){
        return vf2_subgraph_iso(pattern_graph, g, callback,edges_pred,vertex_pred);
    }

private:
  const pattern_graph_type &pattern_graph;
  const result_graph_type &result_graph;
  edges_predicate edges_pred;
  vertex_predicate vertex_pred;
  sub_graph_iso_map_callback callback;
};

template<typename pattern_graph_type,
         typename result_graph_type,
         typename edges_predicate,
         typename vertex_predicate,
         typename sub_graph_iso_map_callback>
auto make_rule(const pattern_graph_type& pattern,
               const result_graph_type& result,
               const sub_graph_iso_map_callback& callback,
               const edges_predicate& e,
               const vertex_predicate& v)
             {
    rule<pattern_graph_type,
         result_graph_type,
            edges_predicate,
            vertex_predicate,
            sub_graph_iso_map_callback> r(pattern,result,e,v,callback);
    return r;
}
template<typename pattern_graph_type,
         typename result_graph_type,
         typename edges_predicate,
         typename vertex_predicate,
         typename sub_graph_iso_map_callback>
auto make_rule(const pattern_graph_type& pattern,
               const result_graph_type& result,
               const sub_graph_iso_map_callback& callback)
             {

    return make_rule(pattern,result,callback,boost::always_equivalent(), boost::always_equivalent());
}

TEST(Backtracking, checkIfDetectIsomorphism) {
    typedef adjacency_list<vecS, vecS, undirectedS,external_data,external_data> graph_type;

    // Build graph1
    graph_type graph1;
    init_triangle(graph1);

    // Build graph2
    graph_type graph2;
    init_diamond(graph2);

    //rule<graph_type,graph_type> transformation(graph1,graph1);

    // Create callback to print mappings
    counter_graph_callback<graph_type, graph_type> callback(graph1, graph2);

    // Print out all subgraph isomorphism mappings between graph1 and graph2.
    // Vertices and edges are assumed to be always equivalent.
    EXPECT_TRUE(vf2_subgraph_iso(graph1, graph2, callback));
}

class SimpleBacktrackingTest : public ::testing::Test {
 protected:
  typedef adjacency_list<vecS, vecS, bidirectionalS,external_data,external_data> graph_type;
    SimpleBacktrackingTest() : handle(repo),repo() {
        FILE* log_fd = fopen( "mylogfile_backtracking.txt", "w" );
        Output2FILE::Stream() = log_fd;
    }
    virtual void SetUp() {
        size = 4;
        for(int i=0; i<size;++i){
            add_vertex(handle.getGraph());
        }
    }
    virtual void TearDown() { }
    void backtrack(list<graph_type>& results, int target_degree){
        typename boost::graph_traits<graph_type>::vertex_iterator vi,vi2, vi_end,vi2_end;
        for (boost::tie(vi, vi_end) = boost::vertices(handle.getGraph()); vi != vi_end; ++vi){
            for (boost::tie(vi2, vi2_end) = boost::vertices(handle.getGraph()); vi2 != vi2_end; ++vi2){
                FILE_LOG(logDEBUG1) << "processing edge: " << *vi << " -> " << *vi2;
                auto rev = handle.get_revision();
                if((*vi==*vi2) || boost::edge(*vi,*vi2,handle.getGraph()).second){
                    continue;
                }
                ASSERT_EQ(size,num_vertices(handle.getGraph()));
                commit(handle);// savepoint
                auto e = add_edge(*vi,*vi2,handle.getGraph());
                ASSERT_TRUE(e.second);
                auto desc = e.first;
                FILE_LOG(logDEBUG1) << "add edge: " << *vi << " -> " << *vi2;
                handle.getGraph()[desc].value = rev.get_rev(); // alter attribute

                int min_degree=10000, max_degree = 0;
                typename boost::graph_traits<graph_type>::vertex_iterator vs,ve;
                for (boost::tie(vs, ve) = boost::vertices(handle.getGraph()); vs != ve; ++vs){
                    int tmp = out_degree(*vs,handle.getGraph());
                    if(tmp<min_degree){
                        min_degree=tmp;
                    }
                    if(tmp>max_degree){
                        max_degree=tmp;
                    }
                }
                FILE_LOG(logDEBUG1) << "max_degree: " << max_degree;
                FILE_LOG(logDEBUG1) << "min_degree: " << min_degree;
                if((max_degree==target_degree)==target_degree){
                    graph_type g(handle.getGraph());
                    results.push_front(g);
                    FILE_LOG(logDEBUG1) << "success, rollback to rev: " << rev;
                    handle = handle.truncate_to(rev);
                }
                ASSERT_EQ(size,num_vertices(handle.getGraph()));
                if(max_degree>target_degree){
                    FILE_LOG(logDEBUG1) << "failed, rollback to rev: " << rev;
                    handle = handle.truncate_to(rev); // rollback
                    ASSERT_EQ(size,num_vertices(handle.getGraph()));
                } else {
                    if(min_degree<target_degree){
                        backtrack(results,target_degree);
                    }
                }
            }
        }
    }
    int backtrack(){
        list<graph_type> results;
        backtrack(results,2);
        cout << "found " << results.size() << " results" << endl;
        return results.size();
    }
    int size;
    archive_handle<graph_type> handle;
    graph_archive<graph_type> repo;
};

TEST_F(SimpleBacktrackingTest, Test1) {
     FILELog::ReportingLevel() = logDEBUG4;
     ASSERT_GE(this->backtrack(),1);
}

TEST(Backtracking, checkSimpleBackTracking) {
    typedef adjacency_list<vecS, vecS, undirectedS,external_data,external_data> graph_type;

    graph_archive<graph_type> repo;
    archive_handle<graph_type> handle(repo);
    init_diamond(handle.getGraph());
    ASSERT_EQ(4,num_vertices(handle.getGraph()));
    ASSERT_EQ(5,num_edges(handle.getGraph()));
    ASSERT_EQ(0,handle.get_revision().get_rev());
    commit(handle);
    ASSERT_EQ(1,handle.get_revision().get_rev());
    add_edge(0,3,handle.getGraph());
    ASSERT_EQ(6,num_edges(handle.getGraph()));

}