#ifndef VERSIONED_GRAPH_H
#define VERSIONED_GRAPH_H
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include "log.h"
#include <stack>
#include <unordered_map>
#include <type_traits>


namespace boost {

namespace detail {

class revision{
    int rev;
    revision(int r) : rev(r){ }
public:
    bool operator<(const revision& r) const{
 //       FILE_LOG(logDEBUG4) << "revision " << rev << " < " << r.rev << " == " << (abs(rev) < abs(r.rev));
        return abs(rev) < abs(r.rev);
    }
    bool operator>(const revision& r) const{
//        FILE_LOG(logDEBUG4) << "revision " << rev << " < " << r.rev << " == " << (abs(rev) < abs(r.rev));
        return abs(rev) > abs(r.rev);
    }
    bool operator<=(const revision& r) const{
//        FILE_LOG(logDEBUG4) << "revision " << rev << " <= " << r.rev << " == " << (abs(rev) <= abs(r.rev));
        return abs(rev) <= abs(r.rev);
    }
    bool operator>=(const revision& r) const{
        return abs(rev) >= abs(r.rev);
    }
    bool operator==(const revision& r) const{
        FILE_LOG(logDEBUG4) << "revision " << rev << " == " << r.rev << " == " << (abs(rev) == abs(r.rev));
        return abs(rev) == abs(r.rev);
    }
    bool is_older(const revision& r)const{
        return *this < r;
    }
    revision& operator++(){
        FILE_LOG(logDEBUG2) << "increment revision";
        assert(rev >=0);
        ++rev;
        FILE_LOG(logDEBUG2) << "incremented";
        return *this;
    }
    revision& operator--(){
        FILE_LOG(logDEBUG2) << "decrement revision";
        assert(rev >0);
        --rev;
        FILE_LOG(logDEBUG2) << "decremented";
        return *this;
    }

//    operator int()
//    {
//         return rev;
//    }
    int get_rev() const {
        return rev;
    }
    static revision create_start(){
        return revision(1);
    }
    static revision create_max(){
        return revision(std::numeric_limits<int>::max());
    }
    static revision create(int value){
        return revision(value);
    }
    revision create_deleted() const{
        revision rev = *this;
        rev.rev = -rev.rev;
        return rev;
    }

};

bool is_deleted(const revision& rev){
    return rev.get_rev()<0;
}
std::ostream& operator<<(std::ostream& os, const revision& obj) {
    return os << obj.get_rev() << " ";
}

/**
 *  Type used for history of vertex and edge bundled properties
 */
template<class T>
struct property_records{
    typedef std::stack<std::pair<revision,T> > type;
};

/**
 *  Type used for history of vertex and edge without bundled properties
 */
template<>
struct property_records<boost::no_property>{
    typedef std::stack<revision> type;
};

/**
 *  Type used for history of graph bundled properties
 */
template<class T>
class property_optional_records{
    typedef std::stack<std::pair<revision,T> > history_type;
    history_type hist;
public:
    void update_if_needed(revision rev,const T& value){
        if(hist.empty()){
            hist.push(std::make_pair(rev,value));
        } else {
            auto p = hist.top();
            assert(p.first<rev);
            if(p.second!=value)
            {
                hist.push(std::make_pair(rev,value));
            }
        }
    }
    void clean_to_max(const revision& rev){
        while(!hist.empty() && hist.top().first>=rev){
            hist.pop();
        }
    }
    void clear(){
        auto prop = hist.top().second;
        while(!hist.empty()){
            hist.pop();
        }
        hist.push(std::make_pair(revision::create_start(),prop));
    }
    T get_latest() const{
        return hist.top().second;
    }

};

/**
 *  Type used for history when there is no graph bundled properties
 */
template<>
struct property_optional_records<boost::no_property>{
    void update_if_needed(revision ,const boost::no_property& ){}
    void clean_to_max(const revision& ){}
    void clear(){}
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
    return value.first;
}
revision get_revision(const revision& value){
    return value;
}

/**
 *  edge_descriptor wrapper used to generate hash, used as key in edge history
 */
template<typename graph>
struct edge_descriptor_cont : graph::edge_descriptor {
    typedef typename graph::edge_descriptor edge_descriptor;
    typedef typename graph::vertex_descriptor vertex_descriptor;
    edge_descriptor_cont(const edge_descriptor& edge,const graph& g) :
        graph::edge_descriptor(edge),
        h(boost::hash<vertex_descriptor>()(boost::source(edge,g))^
        boost::hash<vertex_descriptor>()(boost::target(edge,g)))
    {}
    std::size_t h;
};


template<typename edge_descriptor_cont>
struct edge_hash{
    typedef typename edge_descriptor_cont::vertex_descriptor vertex_descriptor;
    std::size_t operator()(const edge_descriptor_cont& d) const {
          return d.h;
        }

};

template<typename graph_type,typename value_type>
struct filter_removed_predicate{
    revision rev;
    const graph_type* g;
public:
    filter_removed_predicate() : rev(revision::create_start()),g(nullptr) {}
    filter_removed_predicate(const filter_removed_predicate &p) : rev(p.rev),g(p.g){}
    filter_removed_predicate& operator=(const filter_removed_predicate& other) { // copy assignment
        if (this != &other) { // self-assignment check expected
            this->rev = other.rev;
            this->g = other.g;
        }
        return *this;
    }
    filter_removed_predicate(const graph_type* graph,const revision& r = revision::create_max()) : rev(r),g(graph) {}
    bool operator()(const value_type& v) {
        if(rev==revision::create_start() && g==nullptr){
            FILE_LOG(logDEBUG4) << "default filter for edge, match all";
            return true;
        }
        auto list = g->get_history(v);
        revision r = detail::get_revision(list.top());
        assert(r<=g->get_current_rev() && "Top of history is above current rev");
        assert(r<=rev && "Top of history is above wanted rev");
        if(is_deleted(r)){
             FILE_LOG(logDEBUG4) << "filter_predicate: (" << boost::source(v,*g) << ", " << boost::target(v,*g)<< ") found deleted rev: " << r;
             return false;
        }
        FILE_LOG(logDEBUG4) << "filter_predicate: (" << boost::source(v,*g) << ", " << boost::target(v,*g)<< ") found rev: " << r;
        return true;
    }
};
template<typename graph_type>
struct filter_removed_predicate<graph_type,typename boost::graph_traits<graph_type>::vertex_descriptor>{
    revision rev;
    typedef typename boost::graph_traits<graph_type>::vertex_descriptor value_type;
    const graph_type* g;
public:
    filter_removed_predicate() : rev(revision::create_start()),g(nullptr) {}
    filter_removed_predicate(const filter_removed_predicate &p) : rev(p.rev),g(p.g){}
    filter_removed_predicate& operator=(const filter_removed_predicate& other) { // copy assignment
        if (this != &other) { // self-assignment check expected
            this->rev = other.rev;
            this->g = other.g;
        }
        return *this;
    }
    filter_removed_predicate(const graph_type* graph,const revision& r = revision::create_max()) : rev(r),g(graph) {}
    bool operator()(const value_type& v) {
        if(rev==revision::create_start() && g==nullptr){
            FILE_LOG(logDEBUG4) << "default filter for vertex: " << v << ", match all";
            return true;
        }
        FILE_LOG(logDEBUG4) << "filter_removed_predicate: check vertex: " << v;

        auto list = g->get_history(v);
        revision r = detail::get_revision(list.top());
        assert(r<=g->get_current_rev() && "Top of history is above current rev");
        assert(r<=rev && "Top of history is above wanted rev");
        if(is_deleted(r)){
             FILE_LOG(logDEBUG4) << "filter_removed_predicate: found deleted rev: " << r << " and while wanted: " << rev << " desc: " << v;
             return false;
         }
         FILE_LOG(logDEBUG4) << "filter_removed_predicate: found rev: " << r << " and while wanted: " << rev << " desc: " << v;
         return true;
    }
};

template<typename graph_type, typename vertex_descriptor,bool inv>
struct adjacency_filter_removed_predicate{
    revision rev;
    vertex_descriptor u;
    typedef typename boost::graph_traits<graph_type>::vertex_descriptor value_type;
    const graph_type* g;
public:
    adjacency_filter_removed_predicate() : rev(revision::create_start()),u(),g(nullptr) {}
    adjacency_filter_removed_predicate(const adjacency_filter_removed_predicate &p) : rev(p.rev),u(p.u),g(p.g){}
    adjacency_filter_removed_predicate(const graph_type* graph,vertex_descriptor u,const revision& r = revision::create_max()) : rev(r),u(u),g(graph) {}
    bool operator()(const value_type& v) {
        if(rev==revision::create_start() && g==nullptr){
            FILE_LOG(logDEBUG4) << "default filter for vertex: " << v << ", match all";
            return true;
        }
        FILE_LOG(logDEBUG4) << "adjacency_filter_removed_predicate: is_inv("<< inv <<") check edge : v= " << v << " u= " << u;
        auto p = inv ? boost::edge(v,u,g->get_base_graph()) : boost::edge(u,v,g->get_base_graph());
        assert(p.second && "Wanted edge does not exist");
        auto edge_desc = p.first;
        auto list = g->get_history(edge_desc);
        revision r = detail::get_revision(list.top());
        assert(r<=rev && "Top of history is above wanted rev");
        if (is_deleted(r)) {
           FILE_LOG(logDEBUG4) << "adjacency_filter_removed_predicate: found deleted rev: " << r << " and while wanted: " << rev << " desc: " << v;
           return false;
        }
        FILE_LOG(logDEBUG4) << "adjacency_filter_removed_predicate: found rev: " << r << " and while wanted: " << rev << " desc: " << v;
        return true;
    }
};

template<typename unknown_graph_type>
struct graph_tr{
};

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
    typedef detail::revision revision;

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
    typedef typename detail::edge_descriptor_cont<self_type> edge_key;

    typedef detail::filter_removed_predicate<self_type,vertex_descriptor> vertex_predicate;
    typedef detail::filter_removed_predicate<self_type,edge_descriptor> edge_predicate;
    typedef detail::adjacency_filter_removed_predicate<self_type,vertex_descriptor,false> adjacency_predicate;
    friend vertex_predicate;
    friend edge_predicate;
    friend adjacency_predicate;

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
                                    adjacency_predicate,
                                    typename boost::graph_traits<graph_type>::adjacency_iterator>
                             adjacency_iterator;

    typedef detail::vertex_data<vertices_history_type,degree_size_type,directed_category> vertex_stored_data;

    auto vertices_begin() const;
    auto vertices_end() const;
    auto edges_begin() const;
    auto edges_end() const;

    versioned_graph() : direct_base(0,graph_bundled()),vertex_count(0),edge_count(0),current_rev(revision::create_start()) {}
    versioned_graph(vertices_size_type n, const graph_bundled& p = graph_bundled()) : direct_base(n,p),vertex_count(n),edge_count(0),current_rev(revision::create_start()) {
        std::pair<vertex_iterator, vertex_iterator> vi = vertices(get_base_graph());
        for(vertex_iterator it = vi.first; it!=vi.second;++it){
            init(*it);
        }
    }

    versioned_graph(const versioned_graph& g );

    template <class EdgeIterator>
    versioned_graph(EdgeIterator first, EdgeIterator last,
                   vertices_size_type n,
                   edges_size_type m = 0,
                   const graph_bundled& p = graph_bundled()) :  direct_base(first,last,n,m,p),
                                                                vertex_count(n),edge_count(m),current_rev(revision::create_start()) {
        typedef typename graph_type::vertex_iterator v_iter_type;
        std::pair<v_iter_type, v_iter_type> vi = vertices(get_base_graph());
        for(v_iter_type it = vi.first; it!=vi.second;++it){
            init(*it);
        }
        degree_size_type i = 0;
        typedef typename graph_type::edge_iterator e_iter_type;
        std::pair<e_iter_type, e_iter_type> ei = edges(get_base_graph());
        for(e_iter_type it = ei.first; it!=ei.second;++it){
            init(*it);
            ++i;
        }
        edge_count= i;

    }
    /**
     * method used in add_vertex()
     */
    vertex_descriptor generate_vertex(vertex_bundled prop);

    /**
     * method used in add_edge()
     */
    std::pair<edge_descriptor,bool> generate_edge(edge_bundled prop,vertex_descriptor u, vertex_descriptor v);

    void set_deleted(edge_descriptor e);
    void set_deleted(out_edge_iterator e);
    void set_deleted(vertex_descriptor v);
    void remove_permanently(edge_descriptor e);
    void remove_permanently(out_edge_iterator it);
    void remove_permanently(vertex_descriptor e);

    /**
     * Checks if given vertex is deleted
     */
    bool check_if_currently_deleted(vertex_descriptor d) const {
        return is_deleted(detail::get_revision(get_history(d).top()));
    }
    /**
     * Checks if given edge is deleted
     */
    bool check_if_currently_deleted(edge_descriptor d) const {
        return is_deleted(detail::get_revision(get_history(d).top()));
    }

    vertices_size_type num_vertices() const {
        FILE_LOG(logDEBUG4) << "get num vertices " << vertex_count;
        return vertex_count;
    }
    edges_size_type num_edges() const {
        FILE_LOG(logDEBUG4) << "get num edges " << edge_count;
        return edge_count;
    }
    void commit();
    void undo_commit();
    void erase_history();

    void revert_uncommited(){
        FILE_LOG(logDEBUG4) << "revert not commited changes";
        clean_edges_to_current_rev();
        clean_vertices_to_current_rev();
        (*this)[graph_bundle] = graph_bundled_history.get_latest();
    }
    template<typename descriptor>
    revision get_latest_revision(const descriptor& v) const {
        const auto& list = get_history(v);
        return detail::get_revision(list.top());
    }

    graph_type& get_base_graph() {
        return *const_cast<graph_type*>(dynamic_cast<const graph_type*>(this));
    }
    const graph_type& get_base_graph() const{
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

protected:
    void init(vertex_descriptor v, const vertex_bundled& prop = vertex_bundled());
    void init(edge_descriptor e, const edge_bundled& prop = edge_bundled());
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

    edges_history_type& get_history(edge_descriptor idx){
        edge_key key(idx,*this);
        auto iter = edges_history.find(key);
        assert(iter!=edges_history.end());
        return iter->second;
    }
public:
    const vertices_history_type& get_history(vertex_descriptor idx)const {
        return get_stored_data(idx).hist;
    }

    const edges_history_type& get_history(edge_descriptor idx)const {
        edge_key key(idx,*this);
        auto iter = edges_history.find(key);
        assert(iter!=edges_history.end());
        return iter->second;
    }
protected:
    /**
     *  mark edge/vertex as deleted, creates new entry in history
     */
    template<typename descriptor,typename bundled_type>
    void mark_deleted(descriptor e,bundled_type dummy_value){
        using namespace detail;
        auto& list = get_history(e);
        FILE_LOG(logDEBUG4) << "set deleted: " << list.size() << " records in history";
        assert(!is_deleted(detail::get_revision(list.top())));
        revision r = current_rev.create_deleted();
        FILE_LOG(logDEBUG4) << "negated to " << r;
        list.push(make_entry(r,dummy_value));
    }



    template<typename graph,typename descriptor_type,typename property_type>
    struct property_handler{
        static auto& get_latest_bundled_value(const descriptor_type& d, graph& g) {
            auto& list = g.get_history(d);
            assert(detail::get_revision(list.top())<=g.get_current_rev());
            return list.top().second;
        }

        static const auto& get_latest_bundled_value(const descriptor_type& d, const graph& g) {
            const auto& list = g.get_history(d);
            assert(detail::get_revision(list.top())<=g.get_current_rev());
            return list.top().second;
        }

        inline static bool is_update_needed(const descriptor_type& d,const graph& g, const property_type& new_val){
            return get_latest_bundled_value(d,g)!=new_val;
        }
        template<typename value_type>
        static void set_revision(const revision& r, value_type& value){
            value.first = r;
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
        template<typename value_type>
        static void set_revision(const revision& , value_type& ){
        }
    };

    void decr_degree(edge_descriptor e);

    void incr_degree(edge_descriptor e);

    void clean_history( edges_history_type& hist, edge_descriptor desc);

    void clean_history( vertices_history_type& hist, vertex_descriptor desc);

    void clean_edges_to_current_rev();

    void clean_vertices_to_current_rev();
private:
    std::unordered_map<vertex_descriptor,vertex_stored_data,boost::hash<vertex_descriptor> > vertices_history;
    std::unordered_map<edge_key,edges_history_type,detail::edge_hash<edge_key> > edges_history;
    graph_properties_history_type graph_bundled_history;
    vertices_size_type vertex_count;
    edges_size_type edge_count;
    revision current_rev;
};

namespace detail {


/**
 * Base type of versioned graph providing inv_adjacency_iterator type definition
 * in case adjacency list is parameter
 */
template<typename OutEdgeList, typename VertexList,typename  Directed,
         typename VertexProperties, typename EdgeProperties,
         typename GraphProperties, typename EdgeList>
struct graph_tr<boost::versioned_graph<boost::adjacency_list<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>>> : public boost::adjacency_list<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList>{
    typedef typename boost::adjacency_list<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;
    typedef typename boost::graph_bundle_type<graph_type>::type graph_bundled;
    typedef typename boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<graph_type>::vertices_size_type vertices_size_type;
    typedef typename boost::graph_traits<graph_type>::edges_size_type edges_size_type;
    typedef typename boost::detail::adjacency_filter_removed_predicate<versioned_graph<graph_type>,typename graph_type::vertex_descriptor,true> inv_adjacency_predicate;
    typedef typename boost::filter_iterator<
                                    inv_adjacency_predicate,
                                    typename graph_type::inv_adjacency_iterator>
                             inv_adjacency_iterator;
    graph_tr(typename graph_type::vertices_size_type n, const graph_bundled& p = graph_bundled()) : graph_type(n,p){
    }
    template <class EdgeIterator>
    graph_tr(EdgeIterator first, EdgeIterator last,
                   vertices_size_type n,
                   edges_size_type m = 0,
                   const graph_bundled& p = graph_bundled()) :graph_type(first,last,n,m,p) {}
};

/**
 * Base type of versioned graph that lacks inv_adjacency_iterator type definition
 * in case adjacency matrix is parameter
 */
template<typename Directed, typename VertexProperty,
         typename EdgeProperty, typename GraphProperty,
         typename Allocator>
struct graph_tr<boost::versioned_graph<boost::adjacency_matrix<Directed,VertexProperty,EdgeProperty,GraphProperty,Allocator>>> : public boost::adjacency_matrix<Directed,VertexProperty,EdgeProperty,GraphProperty,Allocator> {
    typedef typename boost::adjacency_matrix<Directed,VertexProperty,EdgeProperty,GraphProperty,Allocator> graph_type;
    typedef typename boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_bundle_type<graph_type>::type graph_bundled;
    typedef typename boost::graph_traits<graph_type>::vertices_size_type vertices_size_type;
    typedef typename boost::graph_traits<graph_type>::edges_size_type edges_size_type;

    graph_tr(typename graph_type::vertices_size_type n,const graph_bundled& p = graph_bundled()) : graph_type(n,p){
    }
    template <class EdgeIterator>
    graph_tr(EdgeIterator first, EdgeIterator last,
                   vertices_size_type n,
                   edges_size_type m = 0,
                   const graph_bundled& p = graph_bundled()) :graph_type(first,last,n,m,p) {}
};

}

}
#include "versioned_graph_impl.h"
#include "versioned_graph_non_members.h"
#endif // VERSIONED_GRAPH_H
