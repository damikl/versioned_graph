#ifndef KEY_H
#define KEY_H
#include "log.h"
#include "utils.h"

template< typename T >
struct key_type{
    typedef T vertex_type;
};

template< typename T >
struct key_type<std::pair<T,T> >{
    typedef T vertex_type;
};

/**
    descriptor_typename  -vertex or edge
**/
template<typename descriptor_typename>
struct vertex_id{
    typedef descriptor_typename descriptor_type;
//    typedef typename key_type<descriptor_type>::vertex_type vertex_type;
private:
    internal_vertex identifier;
    descriptor_type desc;
    revision rev;
public:
    revision get_revision() const {
        return rev;
    }
    internal_vertex get_identifier() const {
        return identifier;
    }
    descriptor_type get_desc() const {
        return desc;
    }
    void set_revision(int rev) {
        this->rev = rev;
    }
    void set_identifier(int ident) {
        this->identifier =  ident;
    }
    void set_desc(const descriptor_type& d) {
        this->desc = d;
    }
    vertex_id(descriptor_type desc, int rev,internal_vertex ident) : identifier(ident),desc(desc),rev(rev){
    }
    vertex_id(const vertex_id& key) : identifier(key.identifier),desc(key.desc),rev(key.rev){
    }
    bool operator==(const vertex_id& obj)const{
        if((this->identifier == obj.identifier || obj.identifier < 0|| this->identifier < 0) &&
                this->desc == obj.desc &&
                this->rev.rev == obj.rev.rev)
            return true;
        return false;
    }
    bool operator!=(const vertex_id& obj)const{
        return !(*this ==obj);
    }
    static vertex_id get_max(descriptor_type id){
         vertex_id arch;
         arch.desc = id;

         arch.revision = std::numeric_limits<int>::max();
         return arch;
    }
    static vertex_id get_min(descriptor_type id){
         vertex_id arch;
         arch.desc = id;
         arch.revision = 0;
         return arch;
    }
    bool is_deleted(){
        return int(rev) < 0;
    }
};

template<typename descriptor_typename>
struct edge_id{
    typedef descriptor_typename descriptor_type;
private:
    revision rev;
    descriptor_type desc;
    std::pair<internal_vertex,internal_vertex> identifier;
public:
    revision get_revision() const {
        return rev;
    }
    std::pair<internal_vertex,internal_vertex> get_identifier() const {
        return identifier;
    }
    void set_revision(int rev) {
        this->rev = rev;
    }
    void set_source(const std::pair<internal_vertex,internal_vertex>& ident) {
        this->identifier =  ident;
    }
    edge_id(descriptor_type d,int rev,const std::pair<internal_vertex,internal_vertex>& ident) : rev(rev),desc(d),identifier(ident){
    }
    edge_id(const edge_id& key) : rev(key.rev), desc(key.desc),identifier(key.identifier){
    }
    bool operator==(const edge_id& obj)const{
        if((this->identifier == obj.identifier) &&
                this->rev.rev == obj.rev.rev)
            return true;
        return false;
    }
    bool operator!=(const edge_id& obj)const{
        return !(*this ==obj);
    }
    bool is_deleted(){
        return int(rev) < 0;
    }
    descriptor_type get_desc() const{
        return desc;
    }
    void set_desc(const descriptor_type &value){
        this->desc = value;
    }
};

template<typename descriptor_type>
vertex_id<descriptor_type> create_vertex_id(descriptor_type desc, int rev,int ident=-1){
    return vertex_id<descriptor_type>(desc,rev,ident);
}
template<typename descriptor_type>
vertex_id<descriptor_type> create_vertex_id(descriptor_type desc, int rev,internal_vertex ident){
    return vertex_id<descriptor_type>(desc,rev,ident);
}
template<typename descriptor_type>
edge_id<descriptor_type> create_edge_id(const descriptor_type& desc,int rev,const std::pair<internal_vertex,internal_vertex>& ident){
    return edge_id<descriptor_type>(desc,rev,ident);
}

template<typename key_type>
bool match(const key_type& k1, const key_type& k2){
    if(k1.get_identifier() == k2.get_identifier() || k2.get_identifier() < 0 || k1.get_identifier() < 0){
        if(k1.get_desc() == k2.get_desc()){
            return k1.get_revision() > k2.get_revision();
        } else
             return k1.get_desc() < k2.get_desc();
    } else
        return k1.get_identifier() < k2.get_identifier();
}

template<typename key_type>
typename key_type::descriptor_type::first_type source(const key_type& key){
    return key.get_desc().first;
}

template<typename key_type>
typename key_type::descriptor_type::second_type target(const key_type& key){
    return key.get_desc().second;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::pair<T,T>& obj) {
    return os << obj.first << ", " << obj.second;
}

template<typename Graph>
std::ostream& operator<<(std::ostream& os, const vertex_id<Graph>& obj) {
  // write obj to stream
    return os << obj.get_identifier() << "(" << obj.get_desc() <<") rev: "<< obj.get_revision() <<"";
}
template<typename T>
std::ostream& operator<<(std::ostream& os, const edge_id<T>& obj) {
  // write obj to stream
    return os << "(" << obj.get_identifier() <<") rev: "<< obj.get_revision() <<"";
}
template<typename Graph>
bool thesame(const vertex_id<Graph>& f,const vertex_id<Graph>& s){
    return f.get_identifier() == s.get_identifier() || f.get_desc() == s.get_desc();
}

#endif // KEY_H
