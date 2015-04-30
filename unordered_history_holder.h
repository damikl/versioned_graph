#ifndef UNORDERED_HISTORY_HOLDER_H
#define UNORDERED_HISTORY_HOLDER_H
#include <unordered_map>
#include "utils.h"
#include <list>
#include <algorithm>
#include <boost/iterator/filter_iterator.hpp>

template < typename Type, typename inner_iterator,typename outer_iterator>
class unordered_history_holder_iterator : public std::iterator<std::forward_iterator_tag, typename Type::key_type>{
    typedef std::iterator<std::forward_iterator_tag, typename Type::value_type> iter_type;

    public:
        unordered_history_holder_iterator(Type* cont,outer_iterator o, inner_iterator i): container(cont),
                                                      outer(o), inner(i) {
        }
        unordered_history_holder_iterator(Type* cont,outer_iterator o): container(cont),
                                                      outer(o) {
            outer_iterator oend = container->history_records.end();
            outer_iterator iter;
            if(outer==oend){
                iter = container->history_records.begin();
                inner = iter->second.begin();
            } else {
                inner = outer->second.begin();
            }

        }
        unordered_history_holder_iterator(Type* cont,revision rev): container(cont) {
            outer_iterator oend = container->history_records.end();
            outer_iterator iter = container->history_records.begin();
            while(iter != oend){
                inner_iterator inn_iter = iter->second.begin();
                inner_iterator inn_end = iter->second.end();
                inn_iter = std::lower_bound(inn_iter,inn_end,rev);
                if(inn_iter != inn_end){
                    outer = iter;
                    inner = inn_iter;
                    break;
                }
                ++iter;
            }

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
            if(outer == oend){
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

        const typename Type::value_type get_value_type() const{
            return std::make_pair(outer->first,*inner);
        }
    private:
        outer_iterator outer;
        inner_iterator inner;
        Type* container;
    };

template<typename property_typename>
class vertex{
    typedef property_typename property_type;
private:
    internal_vertex identifier;
    property_type property;
    revision rev;
public:
    revision get_revision() const {
        return rev;
    }
    internal_vertex get_identifier() const {
        return identifier;
    }
    property_type get_property() const {
        return property;
    }
    void set_revision(int rev) {
        this->rev = rev;
    }
    void set_identifier(int ident) {
        this->identifier =  ident;
    }
    void set_property(const property_type& d) {
        this->property = d;
    }
    vertex(property_type p, int rev,internal_vertex ident) : identifier(ident),property(p),rev(rev){
    }
    vertex(const vertex& key) : identifier(key.identifier),property(key.property),rev(key.rev){
    }
    bool operator==(const vertex& obj)const{
        if((this->identifier == obj.identifier) &&
                this->rev.rev == obj.rev.rev)
            return true;
        return false;
    }
    bool operator!=(const vertex& obj)const{
        return !(*this ==obj);
    }
    bool is_deleted(){
        return int(rev) < 0;
    }
};

template<typename property_typename>
class edge{
    typedef property_typename property_type;
private:
    internal_vertex source;
    internal_vertex target;
    property_type property;
    revision rev;
public:
    revision get_revision() const {
        return rev;
    }
    internal_vertex get_source() const {
        return source;
    }
    internal_vertex get_target() const {
        return target;
    }
    property_type get_property() const {
        return property;
    }
    void set_revision(int rev) {
        this->rev = rev;
    }
    void set_source(int ident) {
        this->source =  ident;
    }
    void set_property(const property_type& d) {
        this->property = d;
    }
    edge(property_type p, int rev,internal_vertex ident) : source(ident),property(p),rev(rev){
    }
    edge(const edge& key) : source(key.source),target(key.target),property(key.property),rev(key.rev){
    }
    bool operator==(const edge& obj)const{
        if(this->source == obj.source && this->target == obj.target &&
                this->rev.rev == obj.rev.rev)
            return true;
        return false;
    }
    bool operator!=(const edge& obj)const{
        return !(*this ==obj);
    }
    bool is_deleted(){
        return int(rev) < 0;
    }
};

template<typename property_type>
revision get_revision(const std::pair<revision,property_type>& value){
    return value.first;
}

template<typename descriptor>
revision get_revision(const revision& value){
    return value;
}

template<typename entry>
struct filter_entry_predicate{
    revision rev;
public:
    filter_entry_predicate(): rev(std::numeric_limits<int>::max()){}
    filter_entry_predicate(revision r) : rev(r) {}
    bool operator()(entry v) {
        revision r = get_revision(v);
        bool res = r==rev;
        FILE_LOG(logDEBUG4) << "compare revisions: " << r << " and " << rev << " result: " << res;
        return res;
    }
};

template<typename value_type>
struct filter_predicate{
    revision rev;
    typedef typename value_type::second_type entry;
public:
    filter_predicate(): rev(std::numeric_limits<int>::max()){}
    filter_predicate(const filter_predicate &p) : rev(p.rev){}
    filter_predicate(const revision& r) : rev(r) {}
    bool operator()(const value_type& v) {
        entry list = v.second;
        auto elem = v.first;
        for(auto iter = list.begin(); iter!=list.end();++iter){
            revision r = get_revision(*iter);
            if(r<=rev){
                FILE_LOG(logDEBUG4) << "filter_predicate: found rev: " << r << " and while wanted: " << rev << " for vertex: " << elem;
                return true;
            }
        }
        FILE_LOG(logDEBUG4) << "filter_predicate: not found " << rev << " for vertex: " << elem;
        return false;
    }
};

template<typename property_type, typename param_key_type>
struct unordered_history_holder{
    typedef param_key_type key_type;
    typedef std::pair<revision,property_type> entry;
    typedef std::list<entry> entries;
    typedef typename std::unordered_map<key_type,entries > container;
    typedef unordered_history_holder<property_type,param_key_type> self_type;
    friend class unordered_history_holder_iterator<self_type,typename entries::iterator ,typename container::iterator>;
    friend class unordered_history_holder_iterator<const self_type,typename entries::const_iterator ,typename container::const_iterator>;
//    typedef unordered_history_holder_iterator<self_type,typename entries::iterator ,typename container::iterator> iterator;
//    typedef unordered_history_holder_iterator<const self_type,typename entries::const_iterator ,typename container::const_iterator> const_iterator;

    typedef std::pair<const key_type, entry> value_type;
    typedef boost::filter_iterator<filter_predicate<typename container::value_type>, typename container::iterator> iterator;
    typedef boost::filter_iterator<filter_predicate<typename container::value_type>, typename container::const_iterator> const_iterator;
    typedef typename container::const_iterator outer_const_iterator;
    typedef typename entries::const_iterator inner_const_iterator;

    void insert(key_type key,revision rev,const property_type& property){
        //   int id = get_identifier(key);
        FILE_LOG(logDEBUG4)  << "inserted " << key << " prev size " << size();
        outer_const_iterator iter = history_records.find(key);
        if(iter!=end_full()){
            history_records.at(key).push_front(std::make_pair(rev, property));
        } else {
            entries l;
            l.push_front(std::make_pair(rev, property));
            history_records.insert(std::make_pair(key, l));
        }

    }
/*
    std::pair<inner_const_iterator,bool> find(const key_type& val) const {
        typename container::const_iterator iter = history_records.find(val);
        if(iter != history_records.end()){
            return std::make_pair(iter->begin(),true);
        } else {
            return std::make_pair(iter->end(),false);
        }
    }
*/

    key_type get_key(iterator it) const {
        FILE_LOG(logDEBUG4) << "get_key: " << it->first;
        return it->first;
    }
    key_type get_key(const_iterator it) const {
        FILE_LOG(logDEBUG4) << "get_key: " << it->first;
        return it->first;
    }

    revision get_revision(const_iterator it,const revision& max_rev) const {
        entries e = it->second;
        inner_const_iterator iter = e.begin();
        while(iter != e.end() && max_rev < iter->first){
            ++iter;
        }
        assert(iter != e.end());
        return iter->first;
    }
    template<typename iter>
    std::pair<inner_const_iterator,bool> get_entry(iter it,const revision& rev )const {
        entries l = it->second;
        for(inner_const_iterator i = l.begin(); i != l.end(); ++i){
            if(!(rev<i->first)){
                return std::make_pair(i,true);
            }
        }
        return std::make_pair(l.end(),true);
    }
    template<typename iter>
    std::pair<inner_const_iterator,bool> get_head_entry(iter it)const {
        entries l = it->second;
        if(l.empty()){
            return std::make_pair(l.end(),false);
        } else {
            return std::make_pair(l.begin(),true);
        }
    }
/*
    key_type get_key(const_iterator it) const {
        FILE_LOG(logDEBUG4) << "get_key: " << it.get_value_type().first;
        return it.get_value_type().first;
    }

    revision get_revision(const_iterator it) const {
        entry e = it.get_value_type().second;
        return e.first;
    }
*/
    iterator begin(int rev){
        filter_predicate<typename container::value_type> predicate(revision(rev));
        iterator filter_iter_first(predicate, history_records.begin(), history_records.end());
        return filter_iter_first;
    }
    const_iterator begin(int rev) const{
        revision r(rev);
        filter_predicate<typename container::value_type> predicate(r);
        const_iterator filter_iter_first(predicate, history_records.begin(), history_records.end());
        return filter_iter_first;
    }
    const_iterator cbegin(int rev) const{
        revision r(rev);
        filter_predicate<typename container::value_type> predicate(r);
        const_iterator filter_iter_first(predicate, history_records.cbegin(), history_records.cend());
        return filter_iter_first;
    }
    iterator end(){
        filter_predicate<typename container::value_type> predicate;
        iterator filter_iter_end(predicate, history_records.end(), history_records.end());
        return filter_iter_end;
    }
    const_iterator end() const{
        filter_predicate<typename container::value_type> predicate;
        const_iterator filter_iter_end(predicate, history_records.cend(), history_records.cend());
        return filter_iter_end;
    }
    const_iterator cend() const{
        filter_predicate<typename container::value_type> predicate;
        const_iterator filter_iter_end(predicate, history_records.cend(), history_records.cend());
        return filter_iter_end;
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

//    iterator find(const key_type& val) {
//        return history_records.find(val);
//    }
    outer_const_iterator find(const key_type& val) const {

        return history_records.find(val);
    }

    std::pair<inner_const_iterator,bool> find(const key_type& val,const revision& rev) const {
        typename container::iterator it = history_records.find(val);
        return get_entry(it,rev);
    }

    std::size_t size() const {
        return history_records.size();
    }

private:
    container history_records;
};
/*
template<>
struct unordered_history_holder<boost::no_property>{
    typedef internal_vertex key_type;
    typedef revision entry;
    typedef std::list<entry> entries;
    typedef typename std::unordered_map<internal_vertex,entries > container;
    typedef unordered_history_holder<boost::no_property> self_type;
    friend class unordered_history_holder_iterator<self_type,typename entries::iterator ,typename container::iterator>;
    friend class unordered_history_holder_iterator<const self_type,typename entries::const_iterator ,typename container::const_iterator>;
    typedef unordered_history_holder_iterator<self_type,typename entries::iterator ,typename container::iterator> iterator;
    typedef unordered_history_holder_iterator<const self_type,typename entries::const_iterator ,typename container::const_iterator> const_iterator;

    typedef std::pair<key_type, entry> value_type;
    std::pair<iterator,bool> insert(internal_vertex key, revision rev, const boost::no_property&){
        FILE_LOG(logDEBUG4)  << "inserted " << key << " prev size " << size();
        container::iterator iter = history_records.find(key);
        if(iter != history_records.end()){
            entries& e = iter->second;
            if(e.front()<rev){
                return std::make_pair(end(),false);
            }
            e.push_front(rev);
            return std::make_pair(iterator(this,iter,e.begin()),true);
        } else {
            entries e {rev};
            container::value_type v = std::make_pair(key,e);
            container::iterator it = history_records.insert(v).first;
            return std::make_pair(iterator(this,it,it->second.begin()),true);
        }
    }
    template<typename Graph, typename descriptor>
    static bool changed(descriptor, iterator ,const Graph&) {
        return false;
    }
    template<typename iter>
    key_type get_key(iter it) const {
        FILE_LOG(logDEBUG4) << "get_key: " << *it;
        return *it;
    }

    template<typename iter>
    revision get_revision(iter it) const {
        entry e = it->second;
        return e;
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
*/

#endif // UNORDERED_HISTORY_HOLDER_H
