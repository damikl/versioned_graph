#ifndef VERSIONED_GRAPH_H
#define VERSIONED_GRAPH_H
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include "utils.h"
#include <type_traits>
#include <boost/iterator/filter_iterator.hpp>

namespace boost {

namespace detail {

template<class T>
struct property_records{
    typedef std::list<std::pair<revision,T> > type;
    typedef std::true_type require_comparison;
};

template<>
struct property_records<boost::no_property>{
    typedef std::list<revision> type;
    typedef std::false_type require_comparison;
};

template<class T>
class property_optional_records{
    typedef std::list<std::pair<revision,T> > history_type;
    history_type hist;
public:
    void update_if_needed(revision rev,const T& value){
        if(hist.empty()){
            hist.push_front(std::make_pair(rev,value));
        } else {
            auto p = hist.front();
            assert(p.first<rev);
            if(p.second!=value)
            {
                hist.push_front(std::make_pair(rev,value));
            }
        }
    }
    void clean_to_max(const revision& rev){
        while(!hist.empty() && hist.front().first>=rev){
            hist.pop_front();
        }
    }
    T get_latest() const{
        return hist.front().second;
    }

};

template<>
struct property_optional_records<boost::no_property>{
    void update_if_needed(revision ,const boost::no_property& ){
    }
    void clean_to_max(const revision& ){
    }
    boost::no_property get_latest() const{
        return boost::no_property();
    }
};

template<typename property_type>
std::pair<revision,property_type> make_entry(const revision& r, const property_type& prop){
    return std::make_pair(r,prop);
}

revision make_entry(const revision& r, const no_property& ){
    return r;
}

template<typename property_type>
revision get_revision(const std::pair<revision,property_type>& value){
//    FILE_LOG(logDEBUG4) << "get_revision: " << value.first;
    return value.first;
}
revision get_revision(const revision& value){
//    FILE_LOG(logDEBUG4) << "get_revision: " << value.first;
    return value;
}

template<typename graph_type,typename value_type>
struct filter_predicate{
    revision rev;
    const graph_type* g;
//    typedef typename value_type entry;
public:
    filter_predicate() : rev(std::numeric_limits<int>::max()),g(nullptr) {}
    filter_predicate(const filter_predicate &p) : rev(p.rev),g(p.g){}
    filter_predicate(const graph_type* graph,const revision& r = std::numeric_limits<int>::max()) : rev(r),g(graph) {}
    bool operator()(const value_type& v) {
        if(rev.get_rev()==std::numeric_limits<int>::max() && g==nullptr){
            FILE_LOG(logDEBUG4) << "default filter for edge, match all";
            return true;
        }
        assert(rev>0);
//        FILE_LOG(logDEBUG4) << "filter_predicate: check edge: (" << boost::source(v,*g) << ", " << boost::target(v,*g)<< ")";
        auto list = g->get_history(v);
        for(auto iter = list.begin(); iter!=list.end();++iter){
            revision r = detail::get_revision(*iter);
            assert(r<=g->get_current_rev());
            if(r<=rev){
                if(is_deleted(r)){
                    FILE_LOG(logDEBUG4) << "filter_predicate: (" << boost::source(v,*g) << ", " << boost::target(v,*g)<< ") found deleted rev: " << r;
                    return false;
                }
                FILE_LOG(logDEBUG4) << "filter_predicate: (" << boost::source(v,*g) << ", " << boost::target(v,*g)<< ") found rev: " << r;
                return true;
            } else {
                FILE_LOG(logDEBUG4) << "filter_predicate: (" << boost::source(v,*g) << ", " << boost::target(v,*g)<< ") skipped rev: " << r;
            }
        }
        FILE_LOG(logDEBUG4) << "filter_predicate: not found " << rev << " for edge (" << boost::source(v,*g) << ", " << boost::target(v,*g)<< ")";
        return false;
    }
};
template<typename graph_type>
struct filter_predicate<graph_type,typename boost::graph_traits<graph_type>::vertex_descriptor>{
    revision rev;
    typedef typename boost::graph_traits<graph_type>::vertex_descriptor value_type;
    const graph_type* g;
//    typedef typename value_type entry;
public:
    filter_predicate() : rev(std::numeric_limits<int>::max()),g(nullptr) {}
    filter_predicate(const filter_predicate &p) : rev(p.rev),g(p.g){}
    filter_predicate(const graph_type* graph,const revision& r = std::numeric_limits<int>::max()) : rev(r),g(graph) {}
    bool operator()(const value_type& v) {
        if(rev.get_rev()==std::numeric_limits<int>::max() && g==nullptr){
            FILE_LOG(logDEBUG4) << "default filter for vertex: " << v << ", match all";
            return true;
        }
        assert(rev>0);
        FILE_LOG(logDEBUG4) << "filter_predicate: check vertex: " << v;

        auto list = g->get_history(v);
        for(auto iter = list.begin(); iter!=list.end();++iter){
            revision r = detail::get_revision(*iter);
            assert(r<=g->get_current_rev());
            if(r<=rev){
                if(is_deleted(r)){
                    if(std::is_same<value_type,typename graph_type::vertex_descriptor>::value){
                        FILE_LOG(logDEBUG4) << "filter_predicate: found deleted rev: " << r << " and while wanted: " << rev << " desc: " << v;
                    } else {
                        FILE_LOG(logDEBUG4) << "filter_predicate: found deleted rev: " << r << " and while wanted: " << rev;
                    }
                    return false;
                }
                if(std::is_same<value_type,typename graph_type::vertex_descriptor>::value){
                    FILE_LOG(logDEBUG4) << "filter_predicate: found rev: " << r << " and while wanted: " << rev << " desc: " << v;
                } else {
                    FILE_LOG(logDEBUG4) << "filter_predicate: found rev: " << r << " and while wanted: " << rev;
                }
                return true;
            } else {
                FILE_LOG(logDEBUG4) << "filter_predicate: skipped rev: " << r << " and while wanted: " << rev;
            }
        }
        FILE_LOG(logDEBUG4) << "filter_predicate: not found " << rev;
        return false;
    }
};

template<typename graph_type, typename vertex_descriptor>
struct adjacency_filter_predicate{
    revision rev;
    vertex_descriptor u;
    typedef typename boost::graph_traits<graph_type>::vertex_descriptor value_type;
    const graph_type* g;
    bool inv;
//    typedef typename value_type entry;
public:
    adjacency_filter_predicate() : rev(std::numeric_limits<int>::max()),u(),g(nullptr),inv(false) {}
    adjacency_filter_predicate(const adjacency_filter_predicate &p) : rev(p.rev),u(p.u),g(p.g),inv(p.inv){}
    adjacency_filter_predicate(const graph_type* graph,vertex_descriptor u,bool inv = false,const revision& r = std::numeric_limits<int>::max()) : rev(r),u(u),g(graph),inv(inv) {}
    bool operator()(const value_type& v) {
        if(rev.get_rev()==std::numeric_limits<int>::max() && g==nullptr){
            FILE_LOG(logDEBUG4) << "default filter for vertex: " << v << ", match all";
            return true;
        }
        assert(rev>0);
        FILE_LOG(logDEBUG4) << "adjacency_filter_predicate: is_inv("<< inv <<") check edge : v= " << v << " u= " << u;
        auto p = inv ? boost::edge(v,u,g->get_self()) : boost::edge(u,v,g->get_self());
        assert(p.second);
        auto edge_desc = p.first;
        auto list = g->get_history(edge_desc);
        for(auto iter = list.begin(); iter!=list.end();++iter){
            revision r = detail::get_revision(*iter);
            if(r<=rev){
                if (is_deleted(r)) {
                    FILE_LOG(logDEBUG4) << "adjacency_filter_predicate: found deleted rev: " << r << " and while wanted: " << rev << " desc: " << v;
                    return false;
                }
                FILE_LOG(logDEBUG4) << "adjacency_filter_predicate: found rev: " << r << " and while wanted: " << rev << " desc: " << v;
                return true;
            } else {
                FILE_LOG(logDEBUG4) << "adjacency_filter_predicate: skipped rev: " << r << " and while wanted: " << rev;
            }
        }
        FILE_LOG(logDEBUG4) << "adjacency_filter_predicate: not found " << rev;
        return false;
    }
};

template<typename unknown_graph_type>
struct graph_tr{
};

template<typename versioned_graph,typename bundled_type,typename history_container>
class property_history_reference{
    property_history_reference(versioned_graph& graph) : g(graph),c() {}
    property_history_reference(const property_history_reference& ref) : g(ref.g),c() {}
    property_history_reference& operator=( const property_history_reference& );
public:
    ~property_history_reference(){}
    property_history_reference& operator=( const bundled_type& value ){
        if(detail::get_revision(c.front())< g.current_rev){
            auto p = std::make_pair(g.current_rev,value);
            c.push_front(p);
        } else {
            c.front().second = value;
        }
        return *this;
    }
    operator bundled_type() const{
        return c.front().second();
    }
    versioned_graph& g;
    history_container c;
//    friend class versioned_graph;
};


/*
template<>
struct filter_predicate<std::list<revision> >{
    revision rev;
//    typedef typename value_type entry;
public:
    filter_predicate(): rev(std::numeric_limits<int>::max()){}
    filter_predicate(const filter_predicate &p) : rev(p.rev){}
    filter_predicate(const revision& r) : rev(r) {}
    bool operator()(const std::list<revision>& list) {
        for(auto iter = list.begin(); iter!=list.end();++iter){
            revision r = *iter;
            if(r<=rev){
                FILE_LOG(logDEBUG4) << "filter_predicate: found rev: " << r << " and while wanted: " << rev;
                return true;
            } else {
                FILE_LOG(logDEBUG4) << "filter_predicate: skipped rev: " << r << " and while wanted: " << rev;
            }
        }
        FILE_LOG(logDEBUG4) << "filter_predicate: not found " << rev;
        return false;
    }
};
*/

template<typename vertices_history_type,typename degree_size_type,typename dir_tag>
struct vertex_data{
    vertices_history_type hist;
private:
    degree_size_type out_deg;
public:
    vertex_data():hist(),out_deg(0) {}
    inline degree_size_type incr_out_degree() {
        FILE_LOG(logDEBUG4) << "incr_out_degree " << out_deg;
        return ++out_deg;
    }
    inline void incr_in_degree() {
    }
    inline degree_size_type decr_out_degree() {
        FILE_LOG(logDEBUG4) << "decr_out_degree " << out_deg;
        return --out_deg;
    }
    inline void decr_in_degree() {
    }
    inline degree_size_type get_out_degree() const{
        return out_deg;
    }

//  unavaible
//  inline degree_size_type get_in_degree() const{
//       return 0;
//  }

};
template<typename vertices_history_type,typename degree_size_type>
struct vertex_data<vertices_history_type,degree_size_type,bidirectional_tag>{
    vertices_history_type hist;
private:
    degree_size_type out_deg;
    degree_size_type in_deg;
public:
    vertex_data():hist(),out_deg(0),in_deg(0) {}
    inline degree_size_type incr_out_degree() {
        FILE_LOG(logDEBUG4) << "incr_out_degree " << out_deg;
        return ++out_deg;
    }
    inline degree_size_type incr_in_degree() {
        FILE_LOG(logDEBUG4) << "incr_in_degree " << in_deg;
        return ++in_deg;
    }
    inline degree_size_type decr_out_degree() {
        FILE_LOG(logDEBUG4) << "decr_out_degree " << out_deg;
        return --out_deg;
    }
    inline degree_size_type decr_in_degree() {
        FILE_LOG(logDEBUG4) << "decr_in_degree " << in_deg;
        return --in_deg;
    }
    inline degree_size_type get_out_degree() const{
        return out_deg;
    }
    inline degree_size_type get_in_degree() const{
        return in_deg;
    }
};

}

template<typename graph_t>
class versioned_graph  : public detail::graph_tr<versioned_graph<graph_t>> {
    typedef detail::graph_tr<versioned_graph<graph_t>> direct_base;
public:
    typedef versioned_graph<graph_t> self_type;
    typedef graph_t graph_type;

    typedef typename boost::vertex_bundle_type<graph_type>::type vertex_bundled;
    typedef typename boost::edge_bundle_type<graph_type>::type edge_bundled;
    typedef typename boost::graph_bundle_type<graph_type>::type graph_bundled;
    typedef typename detail::property_records<vertex_bundled>::type vertices_history_type;
    typedef typename detail::property_records<edge_bundled>::type edges_history_type;
    typedef typename detail::property_optional_records<graph_bundled> graph_properties_history_type;

    typedef typename boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<graph_type>::edge_descriptor edge_descriptor;
    typedef typename graph_type::degree_size_type degree_size_type;
    typedef typename graph_type::directed_category directed_category;
    typedef typename graph_type::edge_parallel_category edge_parallel_category;
    typedef typename graph_type::traversal_category traversal_category;
    typedef typename graph_type::vertices_size_type vertices_size_type;
    typedef typename graph_type::edges_size_type edges_size_type;

    typedef detail::filter_predicate<self_type,vertex_descriptor> vertex_predicate;
    typedef detail::filter_predicate<self_type,edge_descriptor> edge_predicate;
    typedef detail::adjacency_filter_predicate<self_type,vertex_descriptor> adjacency_predicate;
    friend vertex_predicate;
    friend edge_predicate;
    friend adjacency_predicate;
//    typedef graph_reference<vertex_bundled,vertex_descriptor> vertex_bundled_reference;
//    typedef graph_reference<edge_bundled,edge_descriptor> edge_bundled_reference;

    typedef typename boost::filter_iterator<
                                    vertex_predicate,
                                    typename boost::graph_traits<graph_type>::vertex_iterator>
                            vertex_iterator;
    typedef typename boost::filter_iterator<
                                    edge_predicate,
                                    typename boost::graph_traits<graph_type>::edge_iterator>
                             edge_iterator;

    typedef typename boost::filter_iterator<
                                    edge_predicate,
                                    typename boost::graph_traits<graph_type>::out_edge_iterator>
                             out_edge_iterator;

    typedef typename boost::filter_iterator<
                                    edge_predicate,
                                    typename boost::graph_traits<graph_type>::in_edge_iterator>
                             in_edge_iterator;

    typedef typename boost::filter_iterator<
                                    detail::adjacency_filter_predicate<self_type,vertex_descriptor>,
                                    typename boost::graph_traits<graph_type>::adjacency_iterator>
                             adjacency_iterator;

    /*
    typedef typename detail::graph_tr<graph_t>::inv_adjacency_iterator inv;
    typedef typename boost::filter_iterator<
                                    detail::adjacency_filter_predicate<self_type,vertex_descriptor>,
                                    typename graph_type::inv_adjacency_iterator>
                             inv_adjacency_iterator;
*/
    typedef detail::vertex_data<vertices_history_type,degree_size_type,directed_category> vertex_stored_data;

    auto vertices_begin() const;
    auto vertices_end() const;
    auto edges_begin() const;
    auto edges_end() const;

    versioned_graph() : direct_base(0,graph_bundled()),v_num(0),e_num(0),current_rev(1) {}
    versioned_graph(vertices_size_type n, const graph_bundled& p = graph_bundled()) : direct_base(n,p),v_num(n),e_num(0),current_rev(1) {
        std::pair<vertex_iterator, vertex_iterator> vi = vertices(get_self());
        for(vertex_iterator it = vi.first; it!=vi.second;++it){
            init(*it);
        }
    }

    versioned_graph(const versioned_graph& g );

    vertex_descriptor generate_vertex(vertex_bundled prop);

    std::pair<edge_descriptor,bool> generate_edge(edge_bundled prop,vertex_descriptor u, vertex_descriptor v);

    void set_deleted(edge_descriptor e);

    template<typename descriptor>
    bool check_if_currently_deleted(descriptor d) const {
        return is_deleted(detail::get_revision(get_history(d).front()));
    }
    void set_deleted(vertex_descriptor v);

    vertices_size_type num_vertices() const {
        FILE_LOG(logDEBUG4) << "get num vertices " << v_num;
        return v_num;
    }
    edges_size_type num_edges() const {
        FILE_LOG(logDEBUG4) << "get num edges " << e_num;
        return e_num;
    }
    void commit();
    void undo_commit();

    void revert_uncommited(){
        FILE_LOG(logDEBUG4) << "revert not commited changes";
        clean_edges_to_current_rev();
        clean_vertices_to_current_rev();
    }
    template<typename descriptor>
    revision get_latest_revision(const descriptor& v) const {
        const auto& list = get_history(v);
        return detail::get_revision(list.front());
    }

    graph_type& get_self() {
        return *const_cast<graph_type*>(dynamic_cast<const graph_type*>(this));
    }
    const graph_type& get_self() const{
        return *dynamic_cast<const graph_type*>(this);
    }

    degree_size_type get_out_degree(vertex_descriptor v) const{
        return get_stored_data(v).get_out_degree();
    }

    degree_size_type get_in_degree(vertex_descriptor v) const{
        return get_stored_data(v).get_in_degree();
    }

    revision get_current_rev() const{
        return current_rev;
    }

private:
    void init(vertex_descriptor v, const vertex_bundled& prop = vertex_bundled());
    vertex_stored_data& get_stored_data(vertex_descriptor u){
        auto iter = vertices_history.find(u);
        assert(iter!=vertices_history.end());
        return iter->second;
    }
    const vertex_stored_data& get_stored_data(vertex_descriptor u) const {
        auto iter = vertices_history.find(u);
        assert(iter!=vertices_history.end());
        return iter->second;
    }

    vertices_history_type& get_history(vertex_descriptor idx){
        return get_stored_data(idx).hist;
    }

    const vertices_history_type& get_history(vertex_descriptor idx)const {
        return get_stored_data(idx).hist;
    }

    edges_history_type& get_history(edge_descriptor idx){
        auto iter = edges_history.find(idx);
        assert(iter!=edges_history.end());
        return iter->second;
    }

    const edges_history_type& get_history(edge_descriptor idx)const {
        auto iter = edges_history.find(idx);
        assert(iter!=edges_history.end());
        return iter->second;
    }

    template<typename descriptor,typename bundled_type>
    void set_deleted(descriptor& e,bundled_type dummy_value){
        using namespace detail;
        auto& list = get_history(e);
        FILE_LOG(logDEBUG4) << "set deleted: " << list.size() << " records in history";
        assert(!is_deleted(detail::get_revision(list.front())));
        revision r = make_deleted(current_rev);
        FILE_LOG(logDEBUG4) << "negated to " << r;
        list.push_front(make_entry(r,dummy_value));
    }

    template<typename descriptor,typename bundled_type>
    void set_latest(descriptor& e,bundled_type value){
        auto& list = get_history(e);
        if(this->get_latest_revision(e)< current_rev){
            auto p = std::make_pair(current_rev,value);
            list.push_front(p);
        } else {
            list.front().second = value;
        }
    }
    template<typename graph,typename descriptor_type,typename property_type>
    struct property_handler{
        static auto& get_latest_bundled_value(const descriptor_type& d, graph& g) {
            auto& list = g.get_history(d);
            assert(detail::get_revision(list.front())<=g.get_current_rev());
            return list.front().second;
        }

        static const auto& get_latest_bundled_value(const descriptor_type& d, const graph& g) {
            const auto& list = g.get_history(d);
            assert(detail::get_revision(list.front())<=g.get_current_rev());
            return list.front().second;
        }

        inline static bool is_update_needed(const descriptor_type& d,const graph& g, const property_type& new_val){
            return get_latest_bundled_value(d,g)!=new_val;
        }

    };
    template<typename graph,typename descriptor_type>
    struct property_handler<graph,descriptor_type,no_property>{
        static no_property get_latest_bundled_value(const descriptor_type& , graph& ) {
            return no_property();
        }
        static no_property get_latest_bundled_value(const descriptor_type& , const graph& ) {
            return no_property();
        }
        inline static bool is_update_needed(const descriptor_type& , const graph&  , no_property& ){
            return false;
        }
    };

    void decr_degree(edge_descriptor e);

    void incr_degree(edge_descriptor e);

    void clean_history( edges_history_type& hist, edge_descriptor desc);

    void clean_history( vertices_history_type& hist, vertex_descriptor desc);

    void clean_edges_to_current_rev();

    void clean_vertices_to_current_rev();

    std::map<vertex_descriptor,vertex_stored_data > vertices_history;
    std::map<edge_descriptor,edges_history_type> edges_history;
    graph_properties_history_type graph_bundled_history;
    vertices_size_type v_num;
    edges_size_type e_num;
    revision current_rev;
};

namespace detail {

template<typename OutEdgeList, typename VertexList,typename  Directed,
         typename VertexProperties, typename EdgeProperties,
         typename GraphProperties, typename EdgeList>
struct graph_tr<boost::versioned_graph<boost::adjacency_list<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>>> : public boost::adjacency_list<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>{
    typedef typename boost::adjacency_list<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;
    typedef typename boost::graph_bundle_type<graph_type>::type graph_bundled;
    typedef typename boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;
    typedef typename boost::filter_iterator<
                                    detail::adjacency_filter_predicate<versioned_graph<graph_type>,typename graph_type::vertex_descriptor>,
                                    typename graph_type::inv_adjacency_iterator>
                             inv_adjacency_iterator;
    graph_tr(typename graph_type::vertices_size_type n, const graph_bundled& p = graph_bundled()) : graph_type(n,p){
    }
};

template<typename Directed, typename VertexProperty,
         typename EdgeProperty, typename GraphProperty,
         typename Allocator>
struct graph_tr<boost::versioned_graph<boost::adjacency_matrix<Directed,VertexProperty,EdgeProperty,GraphProperty,Allocator>>> : public boost::adjacency_matrix<Directed,VertexProperty,EdgeProperty,GraphProperty,Allocator> {
    typedef typename boost::adjacency_matrix<Directed,VertexProperty,EdgeProperty,GraphProperty,Allocator> graph_type;
    typedef typename boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_bundle_type<graph_type>::type graph_bundled;

    graph_tr(typename graph_type::vertices_size_type n,const graph_bundled& p = graph_bundled()) : graph_type(n,p){
    }
};

}

}
#include "versioned_graph_impl.h"
#include "versioned_graph_non_members.h"
#endif // VERSIONED_GRAPH_H
