#ifndef ISOMORPHISM_CHECKING_H
#define ISOMORPHISM_CHECKING_H
#include "mapping.h"
#include <boost/graph/isomorphism.hpp>
#include <functional>

template <typename Graph>
unsigned attribute_vertex_invariant(const typename boost::graph_traits<Graph>::vertex_descriptor &v, const Graph &g){
    typedef typename boost::vertex_bundle_type<Graph>::type vertex_properties;
    vertex_properties prop = g[v];
    std::hash<vertex_properties> h;
    return h(prop);
}

template <typename Graph>
class attribute_vertex_invariant_functor
{
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_t;
    const Graph &graph;
public:
    typedef unsigned int result_type;
    typedef vertex_t argument_type;
    attribute_vertex_invariant_functor(const Graph &g):graph(g){}
    result_type operator()(argument_type v)const
    {
        return (attribute_vertex_invariant(v,graph) ^ boost::in_degree(v,graph)) % max();
    }
    result_type max() const
    {
        return boost::num_vertices(graph)*(boost::num_vertices(graph)+1);
    //    return std::numeric_limits<result_type>::max();
    }
};

template <typename Graph>
attribute_vertex_invariant_functor<Graph> make_attribute_vertex_invariant(const Graph &g) {
    return attribute_vertex_invariant_functor<Graph>(g);
}


enum vertex_desc_t { vertex_desc };
namespace boost {
    BOOST_INSTALL_PROPERTY(vertex, desc);
}

template<typename Graph>
class VertexIndexMap //maps vertex to index
{
public:
    typedef boost::readable_property_map_tag category;
    typedef int  value_type;
    typedef value_type reference;
    typedef typename Graph::vertex_descriptor key_type;

    VertexIndexMap(const mapping<internal_vertex,key_type>& m): map(m) {}
    VertexIndexMap(): map() {}
    mapping<internal_vertex,key_type> map;
    value_type get_value(key_type key) const{
        return map.find(key).first->second.get_identifier();
    }
};

template<typename UniquePairAssociativeContainer>
struct index_map : boost::associative_property_map< UniquePairAssociativeContainer >{
    index_map(){}
    index_map(UniquePairAssociativeContainer map): boost::associative_property_map< UniquePairAssociativeContainer >(m), m(map){
    }
    UniquePairAssociativeContainer m;
};

template<typename Graph>
bool check_isomorphism(const Graph& g1,const Graph& g2) {
    using namespace boost;
    int n = num_vertices(g1);
    if(boost::num_edges(g1)!=boost::num_edges(g2)){
        FILE_LOG(logDEBUG1) << "not isomorphic edges g1= " << boost::num_edges(g1) << " g2= " << boost::num_edges(g2) ;
        return false;
    }
    if(boost::num_vertices(g1)!=boost::num_vertices(g2)){
        FILE_LOG(logDEBUG1) << "not isomorphic, vertices g1= " << boost::num_vertices(g1) << " g2= " << boost::num_vertices(g2) ;
        return false;
    }
//    std::vector<typename graph_traits<Graph>::vertex_descriptor> v1(n), v2(n);

//    typedef std::map<typename graph_traits<Graph>::vertex_descriptor, int> vertex_mapping;
//    vertex_mapping v_indexes1,v_indexes2;
//    boost::associative_property_map< vertex_mapping >
//        v1_index_map(v_indexes1);
//    boost::associative_property_map< vertex_mapping >
//        v2_index_map(v_indexes2);

//    VertexIndexMap<Graph> v1_index_map(g1_mapping);
    FILE_LOG(logDEBUG3) << "get index map";
    typename property_map<Graph, vertex_index_t>::const_type
        v1_index_map = get(vertex_index, g1);

    typename graph_traits<Graph>::vertex_iterator i, end;
    int id = 0;
/*    for (boost::tie(i, end) = vertices(g1); i != end; ++i, ++id) {
        put(v1_index_map, *i, id);
//        v1[id] = *i;
    }
    */
//    int id = 0;
    for (boost::tie(i, end) = vertices(g2); i != end; ++i, ++id) {
        FILE_LOG(logDEBUG3) << "g2: " << *i << " -> " << id;
//        v2[id] = *i;
    }
    id = 0;

    FILE_LOG(logDEBUG4) << "create iso map";
    std::vector<typename graph_traits<Graph>::vertex_descriptor> f(n);
    auto iso_map = make_iterator_property_map(f.begin(), v1_index_map, f[0]);
    FILE_LOG(logDEBUG4) << "calculate isomorphism";
    #if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
      bool ret = isomorphism
        (g1, g2, iso_map,
         degree_vertex_invariant(), get(vertex_index, g1), get(vertex_index, g2));
    #else
      bool ret = isomorphism(g1, g2, isomorphism_map(iso_map).vertex_index1_map(v1_index_map)
                             .vertex_invariant1(make_attribute_vertex_invariant(g1))
                             .vertex_invariant2(make_attribute_vertex_invariant(g2)));
    #endif

    FILE_LOG(logDEBUG1) << "isomorphic? " << ret << " order: ";
    auto tmp = get(vertex_index, g2);
    for (std::size_t v = 0; v != f.size(); ++v){
        FILE_LOG(logDEBUG1) << "g2: " <<f[v] << " -> " <<get(tmp, f[v]);
    }
    return ret;
}

template <typename Graph1, typename Graph2>
struct my_sub_graph_callback {

    my_sub_graph_callback(const Graph1 &graph1, const Graph2 &graph2) : graph1_(graph1), graph2_(graph2) {}

    template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
    bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) const {
        // vf2_graph_iso
        return true;
    }

  private:
    const Graph1 &graph1_;
    const Graph2 &graph2_;
};

template<typename Graph>
bool is_subgraph(const Graph& small, const Graph & large){
    //my_sub_graph_callback<Graph, Graph> my_callback(small, large);
    boost::vf2_print_callback<Graph,Graph>  my_callback(small, large);
    bool res = boost::vf2_graph_iso(small, large, my_callback);
    std::cout << "equal(small, large, my_callback)= " << res << std::endl;
    return res;
}

#endif // ISOMORPHISM_CHECKING_H
