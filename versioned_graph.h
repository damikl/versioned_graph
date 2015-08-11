#ifndef VERSIONED_GRAPH_H
#define VERSIONED_GRAPH_H
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include "utils.h"
#include <type_traits>
#include <boost/iterator/filter_iterator.hpp>

namespace boost {

template<class T>
struct property_records{
    typedef std::list<std::pair<revision,T> > type;
};

template<>
struct property_records<boost::no_property>{
    typedef std::list<revision> type;
};

template<class T>
struct property_optional_records{
    typedef std::list<std::pair<revision,T> > type;
};

template<>
struct property_optional_records<boost::no_property>{
    typedef boost::no_property type;
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
            revision r = get_revision(*iter);
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
            revision r = get_revision(*iter);
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


template<typename versioned_graph,typename bundled_type,typename history_container>
class property_history_reference{
    property_history_reference(versioned_graph& graph) : g(graph),c() {}
    property_history_reference(const property_history_reference& ref) : g(ref.g),c() {}
    property_history_reference& operator=( const property_history_reference& );
public:
    ~property_history_reference(){}
    property_history_reference& operator=( const bundled_type& value ){
        if(get_revision(c.front())< g.current_rev){
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

template<typename OutEdgeList = boost::vecS,
         typename VertexList = boost::vecS,
         typename Directed = boost::directedS,
         typename VertexProperties = boost::no_property,
         typename EdgeProperties = boost::no_property,
         typename GraphProperties = boost::no_property,
         typename EdgeList = boost::listS>
class versioned_graph  : public boost::adjacency_list<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,typename property_optional_records<GraphProperties>::type,EdgeList>{
    template<typename bundled_type,typename descriptor>
    class graph_reference{
        graph_reference(versioned_graph& graph,const descriptor& desc) : g(graph),desc(desc) {}
    public:
        ~graph_reference(){}
        graph_reference& operator=( const bundled_type& x ){
            g.set_latest(desc,x);
            return *this;
        }
        graph_reference& operator=( const graph_reference& x ){
            g.set_latest(desc,g.get_latest_revision(desc),x);
            return *this;
        }
        operator bundled_type&() const{
            return g.get_latest_bundled_value(desc);
        }
        versioned_graph& g;
        descriptor desc;
        friend class versioned_graph;
    };
public:
    typedef typename property_records<VertexProperties>::type vertices_history_type;
    typedef typename property_records<EdgeProperties>::type edges_history_type;
    typedef typename property_optional_records<GraphProperties>::type graph_properties_history_type;
    typedef versioned_graph<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList> self_type;
    typedef property_history_reference<self_type,VertexProperties,vertices_history_type> vertex_property_ref;
    typedef property_history_reference<self_type,EdgeProperties,edges_history_type> edge_property_ref;
    typedef property_history_reference<self_type,GraphProperties,graph_properties_history_type> graph_property_ref;
    typedef boost::adjacency_list<OutEdgeList,VertexList,Directed,VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;

    typedef VertexProperties vertex_bundled;
    typedef EdgeProperties edge_bundled;

    typedef typename boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;
    typedef typename boost::graph_traits<graph_type>::edge_descriptor edge_descriptor;
    typedef typename graph_type::degree_size_type degree_size_type;
    typedef typename graph_type::directed_category directed_category;
    typedef typename graph_type::edge_parallel_category edge_parallel_category;
    typedef typename graph_type::traversal_category traversal_category;
    typedef typename graph_type::vertices_size_type vertices_size_type;
    typedef typename graph_type::edges_size_type edges_size_type;
    typedef graph_reference<vertex_bundled,vertex_descriptor> vertex_bundled_reference;
    typedef graph_reference<edge_bundled,edge_descriptor> edge_bundled_reference;

    typedef typename boost::filter_iterator<
                                    filter_predicate<self_type,vertex_descriptor>,
                                                     typename boost::graph_traits<graph_type>::vertex_iterator>
                                                                                        vertex_iterator;
    typedef typename boost::filter_iterator<
                                    filter_predicate<self_type,edge_descriptor>,
                                                     typename boost::graph_traits<graph_type>::edge_iterator>
                                                                                        edge_iterator;

    typedef typename boost::filter_iterator<
                                    filter_predicate<self_type,edge_descriptor>,
                                                     typename boost::graph_traits<graph_type>::out_edge_iterator>
                                                                                        out_edge_iterator;

    typedef typename boost::filter_iterator<
                                    filter_predicate<self_type,vertex_descriptor>,
                                                     typename boost::graph_traits<graph_type>::adjacency_iterator>
                                                                                        adjacency_iterator;

    auto vertices_begin() const {
        typename graph_type::vertex_iterator iter = boost::vertices(*dynamic_cast<const graph_type*>(this)).first;
        FILE_LOG(logDEBUG4) << "fetched first vertex iterator";
        return iter;
        //return typename graph_type::vertex_iterator(this->vertex_set().begin());
    }

    auto vertices_end() const {
        typename graph_type::vertex_iterator iter = boost::vertices(*dynamic_cast<const graph_type*>(this)).second;
        FILE_LOG(logDEBUG4) << "fetched last vertex iterator";
        return iter;
        //    return typename graph_type::vertex_iterator(this->vertex_set().end());
    }


    auto edges_begin() const {
        typename graph_type::edge_iterator iter = boost::edges(*dynamic_cast<const graph_type*>(this)).first;
        FILE_LOG(logDEBUG4) << "fetched first edge iterator";
        return iter;
        //    return typename graph_type::edge_iterator(this->m_edges.begin());
    }
    auto edges_end() const {
        typename graph_type::edge_iterator iter = boost::edges(*dynamic_cast<const graph_type*>(this)).second;
        FILE_LOG(logDEBUG4) << "fetched last edge iterator";
        return iter;
        //    return typename graph_type::edge_iterator(this->m_edges.end());
    }

    versioned_graph() : /*g(),*/v_num(0),e_num(0),current_rev(1) {}

    versioned_graph(const versioned_graph& g ) : graph_type(),v_num(g.v_num),
                                                 e_num(g.e_num),
                                                 current_rev(g.current_rev) {
        // Would be better to have a constant time way to get from
        // vertices in x to the corresponding vertices in *this.
        std::map<vertex_descriptor,vertex_descriptor> vertex_map;

        // Copy the stored vertex objects by adding each vertex
        // and copying its property object.
        vertex_iterator vi, vi_end;
        vertices_size_type v_count = 0;
        for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
          vertex_descriptor v = add_vertex(get_self());
          (*this)[v] = g[*vi];
          auto iter = g.vertices_history.find(*vi);
          assert(iter!=g.vertices_history.end());
          vertices_history.insert(std::make_pair(v,iter->second));
          if(!is_deleted(get_revision(iter->second.front()))){
              ++v_count;
          }
          vertex_map[*vi] = v;
        }
        assert(v_count==v_num);
        // Copy the edges by adding each edge and copying its
        // property object.
        edge_iterator ei, ei_end;
        edges_size_type e_count = 0;
        for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
          edge_descriptor e;
          bool inserted;
          vertex_descriptor s = source(*ei,g), t = target(*ei,g);
          boost::tie(e, inserted) = add_edge(vertex_map[s],
                                             vertex_map[t], get_self());
          assert(inserted);
          auto iter = g.edges_history.find(*ei);
          assert(iter!=g.edges_history.end());
          edges_history.insert(std::make_pair(e,iter->second));
          (*this)[e] = g[*ei];
          revision r = get_revision(iter->second.front());

          if(!is_deleted(r)){
              ++e_count;
              FILE_LOG(logDEBUG4) << "copy graph: edge marked as existing";
          } else {
              FILE_LOG(logDEBUG4) << "copy graph: edge marked as deleted";
          }
          const edges_history_type& hist = g.get_history(*ei);
          FILE_LOG(logDEBUG4) << "copy graph: copy edge: ("
                              << boost::source(*ei,g)
                              << ", "
                              << boost::target(*ei,g)<< ") "
                              << hist.size() << " records in history, last rev: " << r;
          const edges_history_type& new_hist = get_history(e);
          r = get_revision(new_hist.front());
          FILE_LOG(logDEBUG4) << "copy graph: new edge: ("
                              << boost::source(e,get_self())
                              << ", "
                              << boost::target(e,get_self())<< ") "
                              << new_hist.size() << " records in history, last rev: " << r;
        }
        BOOST_ASSERT_MSG(e_count==e_num,("counted " + std::to_string(e_count) + "  edges while expected " + std::to_string(e_num)).c_str());
        assert(boost::num_vertices(get_self())==vertices_history.size());
        assert(boost::num_edges(get_self())==edges_history.size());
        FILE_LOG(logDEBUG4) << "copied graph: " << v_num << "("<< vertices_history.size() << ") vertices, "
                                                << e_num << "("<< edges_history.size() << ") edges";
    }

    vertex_descriptor generate_vertex(VertexProperties prop){
        vertex_descriptor v = boost::add_vertex(prop,get_self());
        FILE_LOG(logDEBUG4) << "created vertex: " << v;
        vertices_history.insert(std::make_pair(v,vertices_history_type()));
        vertices_history_type& list = get_history(v);
        assert(list.empty());
        auto p = std::make_pair(current_rev,prop);
        list.push_front(p);
        ++v_num;
        return v;
    }

    std::pair<edge_descriptor,bool>
    generate_edge(edge_bundled prop,vertex_descriptor u, vertex_descriptor v){
        auto p = boost::add_edge(u,v,prop,get_self());
        if(p.second){
            if(edges_history.find(p.first)==edges_history.end()){
                edges_history.insert(std::make_pair(p.first,edges_history_type()));
            }
            edges_history_type& list = get_history(p.first);
            assert(list.empty());
            list.push_front(make_entry(current_rev,prop));
            ++e_num;
            FILE_LOG(logDEBUG4) << "add edge (" << boost::source(p.first,get_self()) << ", "
                                                << boost::target(p.first,get_self())
                                                << ") with rev " << current_rev << " num of edges: " << e_num;
        } else {
            FILE_LOG(logDEBUG4) << "edge (" << boost::source(p.first,get_self()) << ", "
                                            << boost::target(p.first,get_self()) << ") already exist";
        }
        return p;
    }
    /*
    std::pair<edge_descriptor,bool>
    generate_edge(vertex_descriptor u, vertex_descriptor v){
        std::pair<edge_descriptor,bool> p = boost::add_edge(u,v,get_self());
        if(p.second){
            if(edges_history.find(p.first)==edges_history.end()){
                edges_history.insert(std::make_pair(p.first,edges_history_type()));
            }
            edges_history_type list = get_history(p.first);
            assert(list.empty());
            list.push_front(make_entry(current_rev,edge_bundled()));
            FILE_LOG(logDEBUG4) << "add edge (" << boost::source(p.first,get_self()) << ", "
                                                << boost::target(p.first,get_self())
                                                << ") with rev " << current_rev;
            ++e_num;
        } else {
            FILE_LOG(logDEBUG4) << "edge (" << boost::source(p.first,get_self()) << ", "
                                            << boost::target(p.first,get_self()) << ") already exist";
        }
        return p;
    }
    */
private:
    template<typename descriptor,typename bundled_type>
    void set_deleted(descriptor& e,bundled_type dummy_value){
        auto& list = get_history(e);
        FILE_LOG(logDEBUG4) << "set deleted: " << list.size() << " records in history";
        assert(!is_deleted(get_revision(list.front())));
        revision r = make_deleted(current_rev);
        FILE_LOG(logDEBUG4) << "negated to " << r;
        auto p = std::make_pair(r,dummy_value);
        FILE_LOG(logDEBUG4) << "entry created";
        list.push_front(p);
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

    vertex_bundled& get_latest_bundled_value(const vertex_descriptor& d) {
        auto& list = get_history(d);
        assert(get_revision(list.front())<=current_rev);
        return list.front().second;
    }

    const vertex_bundled& get_latest_bundled_value(const vertex_descriptor& d) const {
        const auto& list = get_history(d);
        assert(get_revision(list.front())<=current_rev);
        return list.front().second;
    }

    edge_bundled& get_latest_bundled_value(const edge_descriptor& d) {
        auto& list = get_history(d);
        assert(get_revision(list.front())<=current_rev);
        return list.front().second;
    }

    const edge_bundled& get_latest_bundled_value(const edge_descriptor& d) const {
        const auto& list = get_history(d);
        assert(get_revision(list.front())<=current_rev);
        return list.front().second;
    }

public:
    void set_deleted(edge_descriptor e){
        if(get_history(e).size()>1 || get_latest_revision(e) < current_rev){
            set_deleted(e,edge_bundled());
            assert(check_if_currently_deleted(e));
        } else {
            boost::remove_edge(e,*dynamic_cast<graph_type*>(this));
        }
        --e_num;
        FILE_LOG(logDEBUG4) << "set deleted: finnished";
    }
    template<typename descriptor>
    bool check_if_currently_deleted(descriptor d) const {
        return is_deleted(get_revision(get_history(d).front()));
    }
    void set_deleted(vertex_descriptor v){
        if(get_history(v).size()>1 || get_latest_revision(v) < current_rev){
            set_deleted(v,vertex_bundled());
            assert(check_if_currently_deleted(v));
        } else {
            boost::remove_vertex(v,*dynamic_cast<graph_type*>(this));
        }
        --v_num;
        FILE_LOG(logDEBUG4) << "set deleted: finnished";
    }
    vertices_size_type num_vertices() const {
        FILE_LOG(logDEBUG4) << "get num vertices " << v_num;
        return v_num;
    }
    edges_size_type num_edges() const {
        FILE_LOG(logDEBUG4) << "get num edges " << e_num;
        return e_num;
    }
    void commit(){
        auto ei = edges(*this);
        for(auto edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            edges_history_type& hist = get_history(*edge_iter);
                edge_bundled prop = (*this)[*edge_iter];
                if(hist.empty() || get_latest_bundled_value(*edge_iter)!= prop){
                    hist.push_front(make_entry(current_rev,prop));
                }
        }
        auto vi = boost::vertices(*this);
        for(auto vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
            vertices_history_type& hist = get_history(*vertex_iter);
            vertex_bundled prop = (*this)[*vertex_iter];
            if(hist.empty() || get_latest_bundled_value(*vertex_iter)!= prop){
                hist.push_front(make_entry(current_rev,prop));
            }
        }
        ++current_rev;
    }
private:
    void clean_history( edges_history_type& hist, edge_descriptor desc){
        revision r = get_revision(hist.front());
        if (is_deleted(r)) {
           ++e_num;
        }
        FILE_LOG(logDEBUG4) << "remove edges history ( containing " << hist.size() << " records ) entry: ( "
                            << boost::source(desc,*this) << ", "
                            << boost::target(desc,*this) << ") for rev: " << r;
        hist.pop_front();
        if(hist.empty()){
            FILE_LOG(logDEBUG4) << "after edge history removal: ( "
                                << boost::source(desc,*this) << ", "
                                << boost::target(desc,*this) << ") without any history records";
        } else {
            r = get_revision(hist.front());
            FILE_LOG(logDEBUG4) << "after edge history removal: ( "
                                << boost::source(desc,*this) << ", "
                                << boost::target(desc,*this) << ") for rev: " << r;
        }

    }
    void clean_history( vertices_history_type& hist, vertex_descriptor desc){
        revision r = get_revision(hist.front());
        if (is_deleted(r)) {
           ++v_num;
        }
        FILE_LOG(logDEBUG4) << "remove vertices history containing (" << hist.size() << " records ) entry: ("<< desc << ") for rev: " << r;
        hist.pop_front();
        r = get_revision(hist.front());
        FILE_LOG(logDEBUG4) << "after vertices history removal: ("<< desc << ") for rev: " << r;
    }
    std::map<vertex_descriptor,vertices_history_type> vertices_history;
    std::map<edge_descriptor,edges_history_type> edges_history;
public:
    void undo_commit(){
        FILE_LOG(logDEBUG4) << "Undo commit";
        auto ei = boost::edges(get_self());
        std::list<edge_descriptor> edges_to_be_removed;
        for(auto edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
            FILE_LOG(logDEBUG4) << "check edge:( "
                                << boost::source(*edge_iter,*this) << ", "
                                << boost::target(*edge_iter,*this) << ")";
            edges_history_type& hist = get_history(*edge_iter);
            while(!hist.empty() && get_latest_revision(*edge_iter)>=current_rev){
                clean_history(hist,*edge_iter);
            }
            if(hist.empty()){
                edges_to_be_removed.push_front(*edge_iter);
            } else {
                (*this)[*edge_iter] = get_latest_bundled_value(*edge_iter);
                FILE_LOG(logDEBUG4) << "copied edge propety";
            }
        }
        for(edge_descriptor e : edges_to_be_removed){
            auto it = edges_history.find(e);
            assert(it!=edges_history.end());
            edges_history.erase(it);
            --e_num;
            FILE_LOG(logDEBUG4) << "completly removed history record";
            boost::remove_edge(e,get_self());
            FILE_LOG(logDEBUG4) << "removed edge from graph";
        }
        auto vi = boost::vertices(*this);
        std::list<vertex_descriptor> vertices_to_be_removed;
        for(auto vertex_iter = vi.first; vertex_iter != vi.second; ++vertex_iter) {
            vertices_history_type& hist = get_history(*vertex_iter);
            while(!hist.empty() && get_latest_revision(*vertex_iter)>=current_rev){
                clean_history(hist,*vertex_iter);
            }
            if(hist.empty()){
                vertices_to_be_removed.push_front(*vertex_iter);
            } else {
                (*this)[*vertex_iter] = get_latest_bundled_value(*vertex_iter);
                FILE_LOG(logDEBUG4) << "copied vertex propety";
            }
        }
        for(vertex_descriptor v : vertices_to_be_removed){
            auto it = vertices_history.find(v);
            assert(it!=vertices_history.end());
            vertices_history.erase(it);
            --v_num;
            FILE_LOG(logDEBUG4) << "completly removed vertex history record";
            boost::remove_vertex(v,get_self());
            FILE_LOG(logDEBUG4) << "removed vertex from graph";
        }
        --current_rev;
    }
    template<typename descriptor>
    revision get_latest_revision(const descriptor& v) const {
        const vertices_history_type& list = get_history(v);
        return get_revision(list.front());
    }

    graph_type& get_self() {
        return *const_cast<graph_type*>(dynamic_cast<const graph_type*>(this));
    }
    const graph_type& get_self() const{
        return *dynamic_cast<const graph_type*>(this);
    }

    vertices_history_type& get_history(vertex_descriptor idx){
        auto iter = vertices_history.find(idx);
        assert(iter!=vertices_history.end());
        return iter->second;
    }

    const vertices_history_type& get_history(vertex_descriptor idx)const {
        auto iter = vertices_history.find(idx);
        assert(iter!=vertices_history.end());
        return iter->second;
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
/*
    vertex_bundled& operator[](const vertex_descriptor& v) {
    //    graph_reference<vertex_bundled,vertex_descriptor> ref(*this,v);
        return get_latest_bundled_value(v);
    }
    const vertex_bundled& operator[](const vertex_descriptor& v) const {
        return get_latest_bundled_value(v);
    }

    edge_bundled& operator[](const edge_descriptor& e) {
    //    graph_reference<edge_bundled,edge_descriptor> ref(*this,e);
        return get_latest_bundled_value(e);
    }
    const edge_bundled& operator[](const edge_descriptor& e) const {
        return get_latest_bundled_value(e);
    }
*/
    revision get_current_rev()const{
        return current_rev;
    }

private:
//    graph_type g;
    vertices_size_type v_num;
    edges_size_type e_num;
    revision current_rev;
};

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto add_vertex(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.generate_vertex(VertexProperties());
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto add_vertex(VertexProperties p, versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.generate_vertex(p);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto num_vertices(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.num_vertices();
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
void remove_vertex(vertex_descriptor v, versioned_graph<OutEdgeList,VertexList,Directed,
                 VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    g.set_deleted(v);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
void clear_out_edges(vertex_descriptor u, versioned_graph<OutEdgeList,VertexList,Directed,
                     VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
                        VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph;
    auto ei = out_edges(u,g);
    for(typename graph::edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        remove_edge(*edge_iter,g);
    }
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
void clear_in_edges(vertex_descriptor u, versioned_graph<OutEdgeList,VertexList,Directed,
                    VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
                        VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph;
    auto ei = in_edges(u,g);
    for(typename graph::edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
        remove_edge(*edge_iter,g);
    }
}
template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
void clear_vertex(vertex_descriptor u, versioned_graph<OutEdgeList,VertexList,Directed,
                  VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    clear_in_edges(u,g);
    clear_out_edges(u,g);
}


template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto num_edges(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.num_edges();
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto vertices(const versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
            VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;

    filter_predicate<graph_type,typename graph_type::vertex_descriptor> predicate(&g);
    FILE_LOG(logDEBUG4) << "will create begin and end iterators for vertices";
    auto end = g.vertices_end();
    typename graph_type::vertex_iterator iter_begin(predicate, g.vertices_begin(), end);
    typename graph_type::vertex_iterator iter_end(predicate, end, end);
    FILE_LOG(logDEBUG4) << "created begin and end iterators for vertices";
    if(iter_begin!=iter_end){
        FILE_LOG(logDEBUG4) << "first vertex is: " << *iter_begin;
    } else {
        FILE_LOG(logDEBUG4) << "iterators are equal";
    }
    return std::make_pair(iter_begin,iter_end);
}
template<typename T,typename U>
struct assert_same_types{
    typedef T A1;
    typedef U A2;
    static_assert(std::is_same<A1,A2>::value,"Types are not same");
    static void check() {}
};
template<typename From,typename To>
struct assert_convert_types{
    typedef From A1;
    typedef To A2;
    static_assert(std::is_convertible<A1,A2>::value,"Types are not convertible");
    static void check() {}
};

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
auto out_edges(vertex_descriptor u, const versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
            VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;

    filter_predicate<graph_type,typename graph_type::edge_descriptor> predicate(&g);
    auto base_iter_p = out_edges(u,g.get_self());
    typename graph_type::out_edge_iterator iter_begin(predicate, base_iter_p.first, base_iter_p.second);
    typename graph_type::out_edge_iterator iter_end(predicate, base_iter_p.second, base_iter_p.second);
    return std::make_pair(iter_begin,iter_end);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto edges(const versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
            VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;
    FILE_LOG(logDEBUG1) << "get edges";
    filter_predicate<graph_type,typename graph_type::edge_descriptor> predicate(&g);
    typename graph_type::edge_iterator iter_begin(predicate, g.edges_begin(), g.edges_end());
    typename graph_type::edge_iterator iter_end(predicate, g.edges_end(), g.edges_end());
    unsigned int dist = std::distance(iter_begin,iter_end);
    if(dist!=num_edges(g)){
        FILE_LOG(logERROR) << "distance " << dist << " while num_edges: " << num_edges(g);
        exit(1);
    }
    return std::make_pair(iter_begin,iter_end);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
auto adjacent_vertices(vertex_descriptor u,
                  const versioned_graph<OutEdgeList,VertexList,Directed,
                  VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    typedef versioned_graph<OutEdgeList,VertexList,Directed,
            VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;

    auto adj = boost::adjacent_vertices(u,g.get_self());
    filter_predicate<graph_type,typename graph_type::vertex_descriptor> predicate(&g);
    typename graph_type::adjacency_iterator iter_begin(predicate, adj.first, adj.second);
    typename graph_type::adjacency_iterator iter_end(predicate, adj.second, adj.second);
    return std::make_pair(iter_begin,iter_end);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
auto add_edge(vertex_descriptor u,vertex_descriptor v, versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.generate_edge(EdgeProperties(),u,v);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto commit(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.commit();
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList>
auto undo_commit(versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.undo_commit();
}


template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
auto add_edge(vertex_descriptor u,vertex_descriptor v,EdgeProperties p, versioned_graph<OutEdgeList,VertexList,Directed,
                           VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    return g.generate_edge(p,u,v);
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
void remove_edge(vertex_descriptor u,vertex_descriptor v, versioned_graph<OutEdgeList,VertexList,Directed,
                 VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g){
    auto p = edge(u,v,g);
    if(p.second){
        FILE_LOG(logDEBUG4) << "remove_edge: found existing";
        auto edge_desc = p.first;
        FILE_LOG(logDEBUG4) << "remove_edge: got desc: (" << boost::source(edge_desc,g) << ", " << boost::target(edge_desc,g) << ")";
        g.set_deleted(edge_desc);
    }
}

template<typename OutEdgeList,
         typename VertexList,
         typename Directed,
         typename VertexProperties,
         typename EdgeProperties,
         typename GraphProperties,
         typename EdgeList,
         typename vertex_descriptor>
auto edge(vertex_descriptor u,vertex_descriptor v, const versioned_graph<OutEdgeList,VertexList,Directed,
                 VertexProperties,EdgeProperties,GraphProperties,EdgeList>& g) {

    typedef versioned_graph<OutEdgeList,VertexList,Directed,
            VertexProperties,EdgeProperties,GraphProperties,EdgeList> graph_type;
    auto p = boost::edge(u,v,dynamic_cast<const typename graph_type::graph_type&>(g));
    if(p.second){
        FILE_LOG(logDEBUG4) << "edge (" << u << ", " << v  <<  "): found existing intenally";
        auto edge_desc = p.first;
        const typename graph_type::edges_history_type& list = g.get_history(edge_desc);
        FILE_LOG(logDEBUG4) << /*list.size()  <<*/ "records in history of edge";
        if(is_deleted(get_revision(list.front()))){
            FILE_LOG(logDEBUG4) << "edge: is deleted";
            return std::make_pair(typename graph_type::edge_descriptor(),false);
        }
        FILE_LOG(logDEBUG4) << "edge: not deleted";
    } else {
        FILE_LOG(logDEBUG4) << "edge: not found";
    }
    return p;
}

}

#endif // VERSIONED_GRAPH_H
