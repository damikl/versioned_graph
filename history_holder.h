#ifndef HISTORY_HOLDER_H
#define HISTORY_HOLDER_H


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
   std::string to_string()const{
       return std::string("(")+std::string(source) +std::string(",")+ std::string(target) +","+ std::string(revision) +")";
   }
};
template<typename Graph>
std::ostream& operator<<(std::ostream& os, const edge_id<Graph>& obj)
{
  // write obj to stream
    return os << "(" << obj.source << ","<< obj.target<<","<< obj.revision <<")";
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

template<typename key_type,typename property_type>
struct history_holder{
    typedef typename std::map<key_type,property_type> container;
    typedef typename container::iterator iterator;
    typedef typename container::const_iterator const_iterator;
    std::pair<iterator,bool> insert(const key_type& key, const property_type& property){
        return history_records.insert(std::make_pair(key, property));
    }
    template<typename Graph, typename descriptor>
    static bool changed(descriptor v, const_iterator it,const Graph& g) {
        return g[v] != it->second;
    }
    key_type get_key(const_iterator it) const {
        return it->first;
    }
    template<typename Graph>
    void set_edge_property(const_iterator it,
                           Graph& graph,
                           typename key_type::vertex_type s,
                           typename key_type::vertex_type t){
        graph[edge(s, t, graph).first] = it->second;
    }
    iterator begin(){
        return history_records.begin();
    }
    const_iterator begin() const{
        return history_records.begin();
    }
    iterator end(){
        return history_records.end();
    }
    const_iterator end() const{
        return history_records.end();
    }
    bool empty() const{
        return history_records.empty();
    }
    const_iterator upper_bound (const key_type& val) const{
        return history_records.upper_bound(val);
    }

    const_iterator lower_bound (const key_type& val) const{
        return history_records.lower_bound(val);
    }

    iterator upper_bound (const key_type& val) {
        return history_records.upper_bound(val);
    }

    iterator lower_bound (const key_type& val) {
        return history_records.lower_bound(val);
    }

private:
    container history_records;
};

template<typename key_type>
struct history_holder<key_type,boost::no_property>{
    typedef typename std::set<key_type> container;
    typedef typename container::iterator iterator;
    typedef typename container::const_iterator const_iterator;
    std::pair<iterator,bool> insert(const key_type& key, const boost::no_property&){
        return history_records.insert(key);
    }
    template<typename Graph, typename descriptor>
    static bool changed(descriptor, iterator ,const Graph&) {
        return false;
    }
    key_type get_key(const_iterator it) const {
        return *it;
    }
    template<typename Graph>
    void set_edge_property(const_iterator,
                           Graph&,
                           typename key_type::vertex_type,
                           typename key_type::vertex_type){
    }
    iterator begin(){
        return history_records.begin();
    }
    const_iterator begin() const{
        return history_records.begin();
    }
    iterator end(){
        return history_records.end();
    }
    const_iterator end() const{
        return history_records.end();
    }
    bool empty() const{
        return history_records.empty();
    }
    iterator upper_bound (const key_type& val) const{
        return history_records.upper_bound(val);
    }
    iterator lower_bound (const key_type& val) const{
        return history_records.lower_bound(val);
    }
private:
    container history_records;
};

#endif // HISTORY_HOLDER_H
