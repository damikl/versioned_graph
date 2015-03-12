#ifndef KEY_H
#define KEY_H
#include "log.h"

template< typename T >
struct key_type{
    typedef T vertex_type;
};

template< typename T >
struct key_type<std::pair<T,T> >{
    typedef T vertex_type;
};

template<typename descriptor_typename>
struct key_id{
    int identifier;
    typedef descriptor_typename descriptor_type;
    typedef typename key_type<descriptor_type>::vertex_type vertex_type;
    descriptor_type desc;
    int revision;
    key_id(descriptor_type desc, int rev,int ident=-1) : identifier(ident),desc(desc),revision(rev){
        FILE_LOG(logDEBUG4) << "key_id created : " << *this;
    }
//    key_id():revision(0){}
    bool operator<(const key_id& obj)const{
        if(this->identifier == obj.identifier || obj.identifier < 0 || this->identifier < 0){
            if(this->desc == obj.desc)
            {
                return abs(this->revision) > abs(obj.revision);
            } else
                 return this->desc < obj.desc;
        } else
            return this->identifier < obj.identifier;
    }
    bool operator==(const key_id& obj)const{
        if((this->identifier == obj.identifier || obj.identifier < 0|| this->identifier < 0) &&
                this->desc == obj.desc &&
                this->revision == obj.revision)
            return true;
        return false;
    }
    bool operator!=(const key_id& obj)const{
        return !(*this ==obj);
    }
    static key_id get_max(descriptor_type id){
         key_id arch;
         arch.desc = id;
         arch.revision = std::numeric_limits<int>::max();
         return arch;
    }
    static key_id get_min(descriptor_type id){
         key_id arch;
         arch.desc = id;
         arch.revision = 0;
         return arch;
    }
};

template<typename key_type>
typename key_type::vertex_type source(key_type key){
    return key.desc.first;
}

template<typename key_type>
typename key_type::vertex_type target(key_type key){
    return key.desc.second;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::pair<T,T>& obj) {
    return os << obj.first << ", " << obj.second;
}

template<typename Graph>
std::ostream& operator<<(std::ostream& os, const key_id<Graph>& obj) {
  // write obj to stream
    return os << obj.identifier << "(" << obj.desc <<") rev: "<< obj.revision <<"";
}
template<typename Graph>
bool thesame(const key_id<Graph>& f,const key_id<Graph>& s){
    return f.identifier == s.identifier || f.desc == s.desc;
}

#endif // KEY_H
