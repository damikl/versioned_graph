#ifndef SUBGRAPH_TEST_H
#define SUBGRAPH_TEST_H
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>

template <typename Graph1, typename Graph2>
struct counter_graph_callback {
    typedef typename boost::graph_traits<Graph1>::vertex_descriptor graph1_vertex;
    typedef typename boost::graph_traits<Graph2>::vertex_descriptor graph2_vertex;
    counter_graph_callback(const Graph1 &graph1, const Graph2 &graph2) : graph1_(graph1), graph2_(graph2) {
    }
    int get_count() const {
        return count;
    }
    template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
    bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1 g) {
        std::cout << "---START-------------" << std::endl;
        {
            typename boost::graph_traits<Graph1>::vertex_iterator vi, vi_end;
            for (boost::tie(vi, vi_end) = boost::vertices(graph1_); vi != vi_end; ++vi){
                graph1_vertex v = *vi;
                graph2_vertex w = boost::get(f, v);
                if(w!=boost::graph_traits<Graph2>::null_vertex()){
                    std::cout << "vertex: " << v << " from graph1 match to vertex " << w << " in graph2" << std::endl;
                } else {
                    std::cout << "there is no vertex in graph2 like : " << v << " from graph1" << std::endl;
                }
            }
        }
        {
            typename boost::graph_traits<Graph2>::vertex_iterator vi, vi_end;
            for (boost::tie(vi, vi_end) = boost::vertices(graph2_); vi != vi_end; ++vi){
                graph2_vertex w = *vi;
                graph1_vertex v = boost::get(g, w);
                if(v!=boost::graph_traits<Graph1>::null_vertex()){
                    std::cout << "vertex: " << w << " from graph2 match to vertex " << v << " in graph1" << std::endl;
                } else {
                    std::cout << "there is no vertex in graph1 like : " << w << " from graph2" << std::endl;
                }
            }
        }
        std::cout << "---END-------------" << this->get_count() <<  std::endl;
        ++count;

        return true;
    }
    static int count;
  private:
    const Graph1 &graph1_;
    const Graph2 &graph2_;

};
template <typename Graph1, typename Graph2>
int counter_graph_callback<Graph1,Graph2>::count = 0;

struct external_data{
    int value;
    external_data() : value(0) {}
    bool operator==(const external_data& other)const{
        std::cout << "compare: " << this->value << " with " << other.value << std::endl;
        return this->value==other.value;
    }
    bool operator !=(const external_data& other)const{
        return this->value==other.value;
    }
};

std::ostream& operator<<(std::ostream& os, const external_data& obj) {
    return os << obj.value << " ";
}

template<typename graph>
void init_triangle(graph& g){
    auto v0 = boost::add_vertex(g);
    auto v1 = boost::add_vertex(g);
    auto v2 = boost::add_vertex(g);
    auto e0 = boost::add_edge(v0, v1, g).first;
    auto e1 = boost::add_edge(v0, v2, g).first;
    auto e2 = boost::add_edge(v1, v2, g).first;
    g[v0].value = 1;
    g[v1].value = 0;
    g[v2].value = 0;

    g[e0].value = 0;
    g[e1].value = 0;
    g[e2].value = 0;
}

template<typename graph>
void init_diamond(graph& g){
    auto v0 = boost::add_vertex(g);
    auto v1 = boost::add_vertex(g);
    auto v2 = boost::add_vertex(g);
    auto v3 = boost::add_vertex(g);
    auto e0 = boost::add_edge(v0, v1, g).first;
    auto e1 = boost::add_edge(v0, v2, g).first;
    auto e2 = boost::add_edge(v1, v2, g).first;
    auto e3 = boost::add_edge(v1, v3, g).first;
    auto e4 = boost::add_edge(v2, v3, g).first;
    g[v0].value = 1;
    g[v1].value = 0;
    g[v2].value = 0;
    g[v3].value = 0;

    g[e0].value = 0;
    g[e1].value = 0;
    g[e2].value = 0;
    g[e3].value = 0;
    g[e4].value = 0;
}


#endif // SUBGRAPH_TEST_H
