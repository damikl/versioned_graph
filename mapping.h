#ifndef MAPPING_H
#define MAPPING_H
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <utility>

template<typename internal_t,typename external_t>
class mapping{
    typedef internal_t internal_type;
    typedef external_t external_type;
    typedef boost::bimap<boost::bimaps::unordered_set_of<internal_type,std::hash<internal_type> >,
                         boost::bimaps::unordered_set_of<external_type,boost::hash<external_type> > > mapping_type;
    typedef typename mapping_type::left_const_iterator internal_const_iterator;
    typedef typename mapping_type::left_iterator internal_iterator;
    typedef typename mapping_type::right_const_iterator external_const_iterator;
    typedef typename mapping_type::right_iterator external_iterator;
public:
    const internal_type get_internal_id(const external_type& id) const {
        return map.right.at(id);
    }
    internal_type get_internal_id(const external_type& id) {
        return map.right.at(id);
    }

    std::pair<internal_iterator,bool> find( const internal_type& id ){
        internal_iterator it = map.left.find(id);
        return std::make_pair(it,it!=map.left.end());
    }

    std::pair<internal_const_iterator,bool> find( const internal_type& id ) const{
        internal_const_iterator it = map.left.find(id);
        return std::make_pair(it,it!=map.left.end());
    }

    std::pair<external_iterator,bool> find( const external_type& id ){
        external_iterator it = map.right.find(id);
        return std::make_pair(it,it!=map.right.end());
    }

    std::pair<external_const_iterator,bool> find( const external_type& id ) const{
        external_const_iterator it = map.right.find(id);
        return std::make_pair(it,it!=map.right.end());
    }

    void insert(const internal_type& internal_id, const external_type& external_id){
        map.insert( typename mapping_type::value_type(internal_id, external_id) );
    }

    bool erase_internal(const internal_type& id){
        std::size_t n = map.left.erase(id);
        if(n==1){
             return true;
        } else {
            assert(n==0);
            return false;
        }
    }
    bool erase_external(const external_type& id){
        std::size_t n = map.right.erase(id);
        if(n==1){
             return true;
        } else {
            assert(n==0);
            return false;
        }
    }
private:
    mapping_type map;
};



#endif // MAPPING_H
