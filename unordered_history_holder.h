#ifndef UNORDERED_HISTORY_HOLDER_H
#define UNORDERED_HISTORY_HOLDER_H
#include <unordered_map>
#include "utils.h"
#include <list>

template < typename Type, typename innner_iterator>
class unordered_history_holder_iterator : public std::iterator<std::forward_iterator_tag, typename Type::key_type>{
        typedef innner_iterator holder;
    typedef std::iterator<std::forward_iterator_tag, typename Type::value_type> iter_type;
    typedef std::list::iterator inner_iterator;
    typedef std::unordered_map::iterator outer_iterator;

    public:
        unordered_history_holder_iterator(Type* cont,outer_iterator o, inner_iterator i): container(cont),
                                                      outer(o), inner(i) {
        }
        unordered_history_holder_iterator() : outer(),inner() {}

        unordered_history_holder_iterator(const unordered_history_holder_iterator& iter) : outer(iter.outer),
                                                                       inner(iter.inner){
        }

        unordered_history_holder_iterator& operator=(const unordered_history_holder_iterator& other){
            if( this == &other ){
                return( *this ) ;
            }
            this->inner = other.inner;
            this->outer = other.outer;
            return( *this );
        }
        bool operator==(const unordered_history_holder_iterator& o) const{
            return o.outer == this->outer && o.inner == this->inner;
        }
        bool operator==(unordered_history_holder_iterator& o){
            return o.outer == this->outer && o.inner == this->inner;
        }
        bool operator!=(const unordered_history_holder_iterator& o) const{
            return ! ( (*this) == o );
        }
        bool operator!=(unordered_history_holder_iterator& o) {
            return ! ( (*this) == o );
        }

        unordered_history_holder_iterator& operator++(){
            outer_iterator oend = this->container->history_records.end();
            if(current == oend){
                throw std::out_of_range("iterator is out or range");
            } else {
                inner_iterator iend = outer->end();
                if(inner != iend){
                    ++inner;
                } else {
                    ++outer;
                    if(outer != oend){
                        inner = outer->begin();
                    }
                }
            }
            return *this;
        }
        unordered_history_holder_iterator& operator++(int){
            unordered_history_holder_iterator& result(*this);
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
        outer_iterator outer;
        inner_iterator inner;
        Type* container;
    };

template<typename property_type>
struct unordered_history_holder{
    typedef internal_vertex key_type;
    typedef std::pair<revision,property_type> entry;
    typedef std::list<entry> entries;
    typedef typename std::unordered_map<internal_vertex,entries > container;
    typedef history_holder<property_type> self_type;
    friend class unordered_history_holder_iterator<self_type,typename container::iterator>;
    friend class unordered_history_holder_iterator<const self_type,typename container::const_iterator>;
    typedef unordered_history_holder_iterator<self_type,typename container::iterator> iterator;
    typedef unordered_history_holder_iterator<const self_type,typename container::const_iterator> const_iterator;

    typedef std::pair<key_type, entry> value_type;
    std::pair<typename container::iterator,bool> insert(key_type key, const property_type& property){
        //   int id = get_identifier(key);
        int id = get_max_identifier();
        key.set_identifier(id);
        FILE_LOG(logDEBUG4)  << "inserted " << key << " prev size " << size();
        return history_records.insert(std::make_pair(key, property));
    }
    template<typename Graph, typename descriptor, typename iter>
    static bool changed(descriptor v, iter it,const Graph& g) {
        return g[v] != it->second;
    }
    template<typename iter>
    key_type get_key(iter it) const {
        FILE_LOG(logDEBUG4) << "get_key: " << it->first;
        return it->first;
    }

    template<typename Graph>
    void set_edge_property(const_iterator it,
                           Graph& graph,
                           const key_type& p)const{
        graph[edge(source(p), target(p), graph).first] = it->second;
    }
    template<typename Graph>
    void set_edge_property(iterator it,
                           Graph& graph,
                           const key_type& p){
        graph[edge(source(p), target(p), graph).first] = it->second;
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
    typename container::iterator begin_full(){
        return history_records.begin();
    }
    typename container::const_iterator begin_full() const{
        return history_records.begin();
    }
    typename container::const_iterator cbegin_full() const{
        return history_records.cbegin();
    }
    typename container::iterator end_full(){
        return history_records.end();
    }
    typename container::const_iterator end_full() const{
        return history_records.end();
    }
    typename container::const_iterator cend_full() const{
        return history_records.cend();
    }
    bool empty() const{
        return history_records.empty();
    }
    iterator find(const key_type& val) {
        return iterator(this,history_records.find(val));
    }
    const_iterator find(const key_type& val) const {
        return const_iterator(this,history_records.find(val));
    }
    std::size_t size() const {
        return history_records.size();
    }

private:
    container history_records;
};

template<>
struct unordered_history_holder<boost::no_property>{
    typedef internal_vertex key_type;
    typedef revision entry;
    typedef std::list<entry> entries;
    typedef typename std::unordered_map<internal_vertex,entries > container;
    typedef unordered_history_holder<boost::no_property> self_type;
    friend class unordered_history_holder_iterator<self_type,typename container::iterator>;
    friend class unordered_history_holder_iterator<const self_type,typename container::const_iterator>;
    typedef unordered_history_holder_iterator<self_type,typename container::iterator> iterator;
    typedef unordered_history_holder_iterator<const self_type,typename container::const_iterator> const_iterator;

    typedef std::pair<key_type, entry> value_type;
    std::pair<typename container::iterator,bool> insert(key_type key, const boost::no_property&){
     //   int id = get_identifier(key);
        int id = get_max_identifier()+1;
        key.set_identifier(id);
        FILE_LOG(logDEBUG4)  << "inserted " << key << " prev size " << size();
        return history_records.insert(key);
    }
    template<typename Graph, typename descriptor>
    static bool changed(descriptor, iterator ,const Graph&) {
        return false;
    }

    int get_max_identifier()const{
        if(size()==0)
            return 1;
        int ident = get_key(history_records.rbegin()).get_identifier();
        FILE_LOG(logDEBUG4)  << "get_max_identifier: " << ident;
        return ident;
    }
    int get_identifier(const key_type& key )const{
        const_iterator it = lower_bound(key);
        if(it == cend())
            return get_max_identifier()+1;
        const key_type k = get_key(it);
        FILE_LOG(logDEBUG4) << "lower_bound: " << k << " for " << key;
        return k.get_identifier()+1;
    }
    template<typename iter>
    key_type get_key(iter it) const {
        FILE_LOG(logDEBUG4) << "get_key: " << *it;
        return *it;
    }

    template<typename Graph>
    void set_edge_property(const_iterator,
                           Graph&,
                           const key_type&)const{
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
    typename container::iterator begin_full(){
        return history_records.begin();
    }
    typename container::const_iterator begin_full() const{
        return history_records.begin();
    }
    typename container::const_iterator cbegin_full() const{
        return history_records.cbegin();
    }
    typename container::iterator end_full(){
        return history_records.end();
    }
    typename container::const_iterator end_full() const{
        return history_records.end();
    }
    typename container::const_iterator cend_full() const{
        return history_records.cend();
    }
    bool empty() const{
        return history_records.empty();
    }
    iterator find(const key_type& val) {
        return iterator(this,history_records.find(val));
    }
    const_iterator find(const key_type& val) const {
        return const_iterator(this,history_records.find(val));
    }
    std::size_t size() const{
        return history_records.size();
    }

private:
    container history_records;
};

#endif // UNORDERED_HISTORY_HOLDER_H
