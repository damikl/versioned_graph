#ifndef HISTORY_HOLDER_H
#define HISTORY_HOLDER_H
#include "log.h"

template<typename Graph>
struct vertex_id{
    typedef typename Graph::vertex_descriptor vertex_type;
    vertex_type id;
    int revision;
    vertex_id(vertex_type id, int rev) : id(id),revision(rev){}
    vertex_id(){}
    bool operator<(const vertex_id& obj)const{
        if(this->id == obj.id)
            return abs(this->revision) > abs(obj.revision);
        return this->id < obj.id;
    }
    bool operator==(const vertex_id& obj)const{
        if(this->id == obj.id && this->revision == obj.revision)
            return true;
        return false;
    }
    static vertex_id get_max(vertex_type id){
         vertex_id arch;
         arch.id = id;
         arch.revision = std::numeric_limits<int>::max();
         return arch;
    }
    static vertex_id get_min(vertex_type id){
         vertex_id arch;
         arch.id = id;
         arch.revision = 0;
         return arch;
    }
};
template<typename Graph>
struct edge_id{
   typedef typename Graph::vertex_descriptor vertex_type;
   vertex_type source;
   vertex_type target;
   int revision;
   edge_id(){}
   edge_id(vertex_type source,
                vertex_type target,
                int revision):  source(source),
                                target(target),
                                revision(revision){}

   bool operator<(const edge_id& obj) const {
       if(this->source == obj.source){
            if(this->target == obj.target)
                return abs(this->revision) > abs(obj.revision);
            return this->target < obj.target;
       }
       return this->source < obj.source;
   }
   bool operator==(const edge_id& obj)const{
       if(this->source == obj.source &&
          this->target == obj.target &&
          this->revision == obj.revision)
            return true;
       return false;
   }
   edge_id& operator=(edge_id rhs)
   {
     this->source = rhs.source;
     this->target = rhs.target;
     this->revision = rhs.revision;
     return *this;
   }
   static edge_id get_max(vertex_type source, vertex_type target){
        edge_id arch;
        arch.source = source;
        arch.target = target;
        arch.revision = std::numeric_limits<int>::max();
        return arch;
   }
   static edge_id get_min(vertex_type source, vertex_type target){
        edge_id arch;
        arch.source = source;
        arch.target = target;
        arch.revision = 0;
        return arch;
   }
};
template<typename Graph>
std::ostream& operator<<(std::ostream& os, const edge_id<Graph>& obj)
{
  // write obj to stream
    return os << "(" << obj.source << ","<< obj.target<<") rev: " << obj.revision <<")";
}
template<typename Graph>
std::ostream& operator<<(std::ostream& os, const vertex_id<Graph>& obj)
{
  // write obj to stream
    return os << "(" << obj.id <<") rev: "<< obj.revision <<"";
}
template<typename Graph>
bool thesame(const vertex_id<Graph>& f,const vertex_id<Graph>& s){
    return f.id == s.id;
}
template<typename Graph>
bool thesame(const edge_id<Graph>& f,const edge_id<Graph>& s){
    return f.source == s.source && f.target == s.target;
}

template<typename Graph, typename vertex_prop_type>
struct helper;

struct exception_range_checking{
    static void range_check(bool out_of_range) { if(out_of_range) throw std::out_of_range("iterator is out or range");  }
};

template < typename Type, typename range_check_policy, typename innner_iterator>
class history_holder_iterator : public std::iterator<std::forward_iterator_tag, typename Type::key_type>{
        typedef innner_iterator holder;
        typedef std::iterator<std::forward_iterator_tag, typename Type::value_type> iter_type;


    public:
        history_holder_iterator(Type* cont, int rev) : container(cont),revision(rev) {
            this->current = this->container->history_records.begin();
            size_t size = this->container->history_records.size();
            FILE_LOG(logDEBUG3) << "history_holder_iterator ctor, container size" << this->container->history_records.size() << " rev " << revision;
            if(size > 1){
                typename Type::key_type curr = this->container->get_key(current);
                FILE_LOG(logDEBUG3) << "history_holder_iterator ctor, head revision" << curr.revision;
                curr.revision = revision;
                current = this->container->history_records.lower_bound(curr);
            }
        }
        history_holder_iterator(Type* cont,holder c): current(c),container(cont) {
            revision = this->container->get_key(current).revision;
        }
        history_holder_iterator() : current(NULL),container(NULL) {}

        history_holder_iterator(const history_holder_iterator& iter) {
            this->current = iter.current;
            this->container = iter.container;
        }


        history_holder_iterator& operator=(const history_holder_iterator& other){
            if( this == &other ){
                return( *this ) ;
            }
            current = other.current ;
            return( *this ) ;
        }
        bool operator==(const history_holder_iterator& o) const{
            return o.current == this->current;
        }
        bool operator==(history_holder_iterator& o){
            return o.current == this->current;
        }
        bool operator!=(const history_holder_iterator& o) const{
            return ! ( (*this) == o );
        }
        bool operator!=(history_holder_iterator& o) {
            return ! ( (*this) == o );
        }

        history_holder_iterator& operator++(){
            range_check_policy::range_check(current == this->container->history_records.end());
            typename Type::key_type curr = this->container->get_key(*this);
            curr.revision = 0;
            current = this->container->history_records.upper_bound(curr);
            curr = this->container->get_key(*this);
            FILE_LOG(logDEBUG3) << "++history_holder_iterator ctor, container size" << this->container->history_records.size() << " rev " << revision;
            curr.revision = revision;
            current = this->container->history_records.lower_bound(curr);
            return *this;
        }
        history_holder_iterator& operator++(int){
            history_holder_iterator& result(*this);
            ++(*this);
            return result;
        }

        typename iter_type::reference operator*() const{
            return const_cast<typename Type::value_type&>(*current);
        }
        typename iter_type::pointer operator->() const{
            return &const_cast<typename Type::value_type&>(*current);
        }
    private:
        holder current;
        Type* container;
        int revision;
    };

template<typename versioned_key_type,typename property_type>
struct history_holder{
    typedef versioned_key_type key_type;
    typedef typename std::map<key_type,property_type> container;
    typedef history_holder<key_type,property_type> self_type;
    friend class history_holder_iterator<self_type,exception_range_checking,typename container::iterator>;
    friend class history_holder_iterator<const self_type,exception_range_checking,typename container::const_iterator>;
    typedef history_holder_iterator<self_type,exception_range_checking,typename container::iterator> iterator;
    typedef history_holder_iterator<const self_type,exception_range_checking,typename container::const_iterator> const_iterator;

    typedef std::pair<const versioned_key_type, property_type> value_type;
    std::pair<typename container::iterator,bool> insert(const key_type& key, const property_type& property){
        return history_records.insert(std::make_pair(key, property));
    }
    template<typename Graph, typename descriptor>
    static bool changed(descriptor v, iterator it,const Graph& g) {
        return g[v] != it->second;
    }
    key_type get_key(typename container::const_iterator it) const {
        return it->first;
    }
    key_type get_key(typename container::iterator it) {
        return it->first;
    }
    key_type get_key(const_iterator it) const {
        return it->first;
    }
    key_type get_key(iterator it) {
        return it->first;
    }
    template<typename Graph>
    void set_edge_property(const_iterator it,
                           Graph& graph,
                           typename key_type::vertex_type s,
                           typename key_type::vertex_type t)const{
        graph[edge(s, t, graph).first] = it->second;
    }
    template<typename Graph>
    void set_edge_property(iterator it,
                           Graph& graph,
                           typename key_type::vertex_type s,
                           typename key_type::vertex_type t){
        graph[edge(s, t, graph).first] = it->second;
    }
    iterator begin(int revision){
        return iterator(this,revision);
    }
    const_iterator begin(int revision) const{
        return const_iterator(this,revision);
    }
    const_iterator cbegin(int revision) const{
        return const_iterator(this,revision);
    }
    iterator end(){
        return iterator(this,history_records.end());
    }
    const_iterator end() const{
        return const_iterator(this,history_records.end());
    }
    const_iterator cend() const{
        return const_iterator(this,history_records.end());
    }
    bool empty() const{
        return history_records.empty();
    }
    const_iterator upper_bound (const key_type& val) const{
        return const_iterator(this,history_records.upper_bound(val));
    }

    const_iterator lower_bound (const key_type& val) const{
        return const_iterator(this,history_records.lower_bound(val));
    }

    iterator upper_bound (const key_type& val) {
        return iterator(this,history_records.upper_bound(val));
    }

    iterator lower_bound (const key_type& val) {
        return iterator(this,history_records.lower_bound(val));
    }

private:
    container history_records;
};

template<typename versioned_key_type>
struct history_holder<versioned_key_type,boost::no_property>{
    typedef versioned_key_type key_type;
    typedef typename std::set<key_type> container;
    typedef history_holder<key_type,boost::no_property> self_type;
    friend class history_holder_iterator<self_type,exception_range_checking,typename container::iterator>;
    friend class history_holder_iterator<const self_type,exception_range_checking,typename container::const_iterator>;
    typedef history_holder_iterator<self_type,exception_range_checking,typename container::iterator> iterator;
    typedef history_holder_iterator<const self_type,exception_range_checking,typename container::const_iterator> const_iterator;
    typedef versioned_key_type value_type;
    std::pair<typename container::iterator,bool> insert(const key_type& key, const boost::no_property&){
        return history_records.insert(key);
    }
    template<typename Graph, typename descriptor>
    static bool changed(descriptor, iterator ,const Graph&) {
        return false;
    }
    key_type get_key(typename container::const_iterator it) const {
        return *it;
    }
    key_type get_key(typename container::iterator it) {
        return *it;
    }
    key_type get_key(const_iterator it) const {
        return *it;
    }
    key_type get_key(iterator it) {
        return *it;
    }
    template<typename Graph>
    void set_edge_property(const_iterator,
                           Graph&,
                           typename key_type::vertex_type,
                           typename key_type::vertex_type)const{
    }
    iterator begin(int revision){
        return iterator(this,revision);
    }
    const_iterator begin(int revision) const{
        return const_iterator(this,revision);
    }
    const_iterator cbegin(int revision) const{
        return const_iterator(this,revision);
    }
    iterator end(){
        return iterator(this,history_records.end());
    }
    const_iterator end() const{
        return const_iterator(this,history_records.end());
    }
    const_iterator cend() const{
        return const_iterator(this,history_records.end());
    }
    bool empty() const{
        return history_records.empty();
    }
    const_iterator upper_bound (const key_type& val) const{
        return const_iterator(this,history_records.upper_bound(val));
    }

    const_iterator lower_bound (const key_type& val) const{
        return const_iterator(this,history_records.lower_bound(val));
    }

    iterator upper_bound (const key_type& val) {
        return iterator(this,history_records.upper_bound(val));
    }

    iterator lower_bound (const key_type& val) {
        return iterator(this,history_records.lower_bound(val));
    }
private:
    container history_records;
};

#endif // HISTORY_HOLDER_H
