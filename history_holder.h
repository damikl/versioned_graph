#ifndef HISTORY_HOLDER_H
#define HISTORY_HOLDER_H
#include "log.h"
#include "key.h"

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
        history_holder_iterator(Type* cont, int _rev) : container(cont),rev(_rev) {
            this->current = this->container->history_records.begin();
            size_t size = this->container->history_records.size();
            FILE_LOG(logDEBUG4) << "history_holder_iterator ctor, container size " << size << " rev " << this->rev;
            if(size > 1) {
                typename Type::key_type curr = this->container->get_key(current);
                FILE_LOG(logDEBUG4) << "history_holder_iterator ctor " <<  this << ", first entry: " << curr;
                curr.set_revision(rev);
                current = this->container->history_records.lower_bound(curr);
            }
        }
        history_holder_iterator(Type* cont,holder c): current(c),container(cont),
                                                      rev(0) {
            rev = container->get_key(current).get_revision();
        }
        history_holder_iterator() : current(NULL),container(NULL),rev(0) {}

        history_holder_iterator(const history_holder_iterator& iter) : current(iter.current),
                                                                       container(iter.container),
                                                                       rev(iter.rev) {
        }

        history_holder_iterator& operator=(const history_holder_iterator& other){
            if( this == &other ){
                return( *this ) ;
            }
            current = other.current ;
            this->container = other.container;
            this->rev = other.rev;
            return( *this );
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
            typedef typename Type::key_type key;
            key curr = this->container->get_key(*this);
            key begin = this->container->get_key(this->container->begin(rev));
            FILE_LOG(logDEBUG4) << "++history_holder_iterator " << this <<", wanted rev: " << rev;
            FILE_LOG(logDEBUG4) << "++history_holder_iterator, was at " << curr;
            holder end = this->container->history_records.end();
            key target(curr.get_desc(),-2,-2);// fake assignment, because there is no default constructor
            do{
                curr.set_revision(0);
                current = this->container->history_records.upper_bound(curr);
                if(current == end)
                    return *this;
                curr = this->container->get_key(*this);
                // first revision of next node
                FILE_LOG(logDEBUG4) << "++history_holder_iterator intermediate " << curr;
                if(curr < begin){
                    FILE_LOG(logDEBUG4) << "end key: " << this->container->get_key(this->container->end());
                    FILE_LOG(logDEBUG4) << "++history_holder_iterator target overflow";
                    break;
                }
                curr.set_revision(rev);
                current = this->container->history_records.lower_bound(curr);
                target = this->container->get_key(current);
                FILE_LOG(logDEBUG4) << "++history_holder_iterator target " << target << " current " << curr << " same ? " << thesame(curr,target);
            } while((!thesame(curr,target) && current != end) || target.is_deleted() );
            FILE_LOG(logDEBUG4) << "++history_holder_iterator finished at " << target;
            FILE_LOG(logDEBUG4) << "--------------------------------------";
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
        revision rev;
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
    int get_max_identifier()const{
        if(size()==0)
            return 1;
        int ident = get_key(history_records.rbegin()).get_identifier()+1;
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


#endif // HISTORY_HOLDER_H
