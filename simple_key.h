#ifndef SIMPLE_KEY_H
#define SIMPLE_KEY_H
#include "key.h"
#include "utils.h"

class simple_key{
    int identifier;
    revision rev;
public:
    revision get_revision() const {
        return rev;
    }
    int get_identifier() const {
        return identifier;
    }
    void set_revision(int rev) {
        this->rev = rev;
    }
    void set_identifier(int ident) {
        this->identifier =  ident;
    }
    key_id(int rev,int ident=-1) : identifier(ident),rev(rev){
        FILE_LOG(logDEBUG4) << "key_id created : " << *this;
    }
    key_id(const simple_key& key) : identifier(key.identifier),rev(key.rev){
//        FILE_LOG(logDEBUG4) << "key_id copied : " << *this;
    }
    bool operator<(const key_id& obj)const{
        if(this->get_identifier() == obj.get_identifier()){
           return this->get_revision() > obj.get_revision();
        } else
            return this->get_identifier() < obj.get_identifier();
    }
    bool operator==(const simple_key& obj)const{
        if((this->identifier == obj.identifier || obj.identifier < 0|| this->identifier < 0) &&
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

#endif // SIMPLE_KEY_H
