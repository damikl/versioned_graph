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
        history_holder_iterator(Type* cont, int rev) : container(cont),revision(rev) {
            this->current = this->container->history_records.begin();
            size_t size = this->container->history_records.size();
            FILE_LOG(logDEBUG4) << "history_holder_iterator ctor, container size " << size << " rev " << revision;
            if(size > 1){
                typename Type::key_type curr = this->container->get_key(current);
                FILE_LOG(logDEBUG4) << "history_holder_iterator ctor, head revision " << curr.revision;
                FILE_LOG(logDEBUG4) << "first entry: " << curr;
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
            this->revision = iter.revision;
        }


        history_holder_iterator& operator=(const history_holder_iterator& other){
            if( this == &other ){
                return( *this ) ;
            }
            current = other.current ;
            this->container = other.container;
            this->revision = other.revision;
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
            typename Type::key_type curr = this->container->get_key(*this);
            FILE_LOG(logDEBUG4) << "++history_holder_iterator, wanted rev: " << revision;
            FILE_LOG(logDEBUG4) << "++history_holder_iterator, was at " << curr;

            typename Type::key_type target = curr;// fake assignment, because there is no default constructor
            target.revision = -2;
            target.identifier = -2;
            do{
                curr.revision = 0;
                current = this->container->history_records.upper_bound(curr);
                if(current == this->container->history_records.end())
                    return *this;
                curr = this->container->get_key(*this);
                FILE_LOG(logDEBUG4) << "++history_holder_iterator intermediate " << curr;
                if(curr < this->container->get_key(this->container->begin(revision))){
                    FILE_LOG(logDEBUG4) << "end key: " << this->container->get_key(this->container->end());
                    FILE_LOG(logDEBUG4) << "++history_holder_iterator target overflow";
                    break;
                }
                curr.revision = revision;
                current = this->container->history_records.lower_bound(curr);
                target = this->container->get_key(current);
                FILE_LOG(logDEBUG4) << "++history_holder_iterator target " << target << " current " << curr;
            } while((!thesame(curr,target) && current != this->container->history_records.end()) || target.revision < 0 );
            FILE_LOG(logDEBUG4) << "++history_holder_iterator finished at " << target;
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
    std::pair<typename container::iterator,bool> insert(key_type key, const property_type& property){
        int id = get_identifier(key);
        key.identifier = id;
        FILE_LOG(logDEBUG4)  << "inserted " << key << " prev size " << size();
        return history_records.insert(std::make_pair(key, property));
    }
    template<typename Graph, typename descriptor>
    static bool changed(descriptor v, iterator it,const Graph& g) {
        return g[v] != it->second;
    }
    template<typename iter>
    key_type get_key(iter it) const {
        return it->first;
    }

    template<typename Graph>
    void set_edge_property(const_iterator it,
                           Graph& graph,
                           key_type p)const{
        graph[edge(source(p), target(p), graph).first] = it->second;
    }
    template<typename Graph>
    void set_edge_property(iterator it,
                           Graph& graph,
                           key_type p){
        graph[edge(source(p), target(p), graph).first] = it->second;
    }
    int get_max_identifier()const{
        if(size()==0)
            return 1;
        FILE_LOG(logDEBUG4)  << "get_max_identifier: " << size();
        return get_key(history_records.rbegin()).identifier;
    }
    int get_identifier(key_type key )const{
        key.identifier = -1;
        const_iterator it = lower_bound(key);
        if(it == cend())
            return get_max_identifier()+1;
        return get_key(it).identifier;
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
        int id = get_identifier(key);
        key.identifier = id;
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
        FILE_LOG(logDEBUG4)  << "get_max_identifier: " << size();
        return get_key(history_records.rbegin()).identifier;
    }
    int get_identifier(key_type key )const{
        key.identifier = -1;
        const_iterator it = lower_bound(key);
        if(it == cend())
            return get_max_identifier()+1;
        return get_key(it).identifier;
    }
    template<typename iter>
    key_type get_key(iter it) const {
        return *it;
    }

    template<typename Graph>
    void set_edge_property(const_iterator,
                           Graph&,
                           key_type)const{
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
