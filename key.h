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

struct revision{
    int rev;
    bool operator<(const revision& r) const{
        return abs(rev) < abs(r.rev);
    }
    bool operator==(const revision& r) const{
        return abs(rev) == abs(r.rev);
    }
    revision& operator++(){
        FILE_LOG(logDEBUG2) << "increment revision";
        assert(rev >=0);
        ++rev;
        FILE_LOG(logDEBUG2) << "incremented";
        return *this;
    }
    revision(int r) : rev(r){ }
    operator int()
    {
         return rev;
    }
    int get_rev() const {
        return rev;
    }

};

/**
    descriptor_typename  -vertex or edge
**/
template<typename descriptor_typename>
struct key_id{
    typedef descriptor_typename descriptor_type;
    typedef typename key_type<descriptor_type>::vertex_type vertex_type;
private:
    int identifier;
    descriptor_type desc;
    revision rev;
public:
    revision get_revision() const {
        return rev;
    }
    int get_identifier() const {
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
    key_id(descriptor_type desc, int rev,int ident=-1) : identifier(ident),desc(desc),rev(rev){
        FILE_LOG(logDEBUG4) << "key_id created : " << *this;
    }
    key_id(const key_id& key) : identifier(key.identifier),desc(key.desc),rev(key.rev){
        FILE_LOG(logDEBUG4) << "key_id copied : " << *this;
    }
//    key_id():revision(0){}
    bool operator<(const key_id& obj)const{
        if(this->identifier == obj.identifier || obj.identifier < 0 || this->identifier < 0){
            if(this->desc == obj.desc)
            {
                return abs(this->rev.get_rev()) > abs(obj.rev.get_rev());
            } else
                 return this->desc < obj.desc;
        } else
            return this->identifier < obj.identifier;
    }
    bool operator==(const key_id& obj)const{
        if((this->identifier == obj.identifier || obj.identifier < 0|| this->identifier < 0) &&
                this->desc == obj.desc &&
                this->rev.rev == obj.rev.rev)
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
    bool is_deleted(){
        return int(rev) < 0;
    }
};

template<typename key_type>
typename key_type::vertex_type source(key_type key){
    return key.get_desc().first;
}

template<typename key_type>
typename key_type::vertex_type target(key_type key){
    return key.get_desc().second;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::pair<T,T>& obj) {
    return os << obj.first << ", " << obj.second;
}

std::ostream& operator<<(std::ostream& os, const revision& obj) {
    return os << obj.get_rev() << " ";
}

template<typename Graph>
std::ostream& operator<<(std::ostream& os, const key_id<Graph>& obj) {
  // write obj to stream
    return os << obj.get_identifier() << "(" << obj.get_desc() <<") rev: "<< obj.get_revision() <<"";
}
template<typename Graph>
bool thesame(const key_id<Graph>& f,const key_id<Graph>& s){
    return f.get_identifier() == s.get_identifier() || f.get_desc() == s.get_desc();
}

#endif // KEY_H
