#ifndef UTILS_H
#define UTILS_H
#include "log.h"
#include "boost/graph/graph_traits.hpp"
#include "boost/graph/adjacency_list.hpp"

#ifndef NDEBUG
#   define MY_ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::exit(EXIT_FAILURE); \
        } \
    } while (false)
#else
#   define MY_ASSERT(condition, message) do { } while (false)
#endif


class internal_vertex {
    int identifier = 0;
    static int counter;
public:
    static internal_vertex create(){
        internal_vertex tmp;
        tmp.identifier = ++counter;
        return tmp;
    }
    friend class std::hash<internal_vertex>;
    friend std::ostream& operator<<(std::ostream& os, const internal_vertex& obj);

    bool operator==(const internal_vertex& v)const{
        return this->identifier==v.identifier;
    }
    bool operator<(const internal_vertex& v)const{
        return this->identifier<v.identifier;
    }
    int get_identifier()const{
        return identifier;
    }
};
int internal_vertex::counter = 0;

namespace std{
    template<>
    struct hash<internal_vertex>
    {
        typedef internal_vertex argument_type;
        typedef size_t result_type;

        result_type operator()(argument_type const& s) const
        {
            return hash<int>()(s.identifier);
        }
    };
    template<>
    struct hash<std::pair<internal_vertex,internal_vertex> >
    {
        typedef std::pair<internal_vertex,internal_vertex> argument_type;
        typedef size_t result_type;

    private:
       hash<internal_vertex> ah;
       hash<internal_vertex> bh;
    public:
       hash() : ah(), bh() {}
       result_type operator()(const argument_type &p) const {
          return ah(p.first) ^ bh(p.second);
       }
    };
    /*
    template<typename Graph>
    struct hash<std::pair<typename boost::graph_traits<Graph>::vertex_descriptor,
            typename boost::graph_traits<Graph>::vertex_descriptor> >
    {
        typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
        typedef std::pair<vertex_descriptor,vertex_descriptor> argument_type;
        typedef size_t result_type;

    private:
       hash<vertex_descriptor> ah;
       hash<vertex_descriptor> bh;
    public:
       hash() : ah(), bh() {}
       result_type operator()(const argument_type &p) const {
          return ah(p.first) ^ bh(p.second);
       }
    };
    */
    string to_string(const pair<internal_vertex,internal_vertex>& v){
        return "( " + to_string(v.first.get_identifier()) + " , " + to_string(v.second.get_identifier()) + " )";
    }
    string to_string(const internal_vertex& v){
        return to_string(v.get_identifier());
    }
}

std::ostream& operator<<(std::ostream& os, const internal_vertex& obj) {
    return os << obj.identifier << " ";
}



struct revision{
    int rev;
    bool operator<(const revision& r) const{
        FILE_LOG(logDEBUG4) << "revision " << rev << " < " << r.rev << " == " << (abs(rev) < abs(r.rev));
        return abs(rev) < abs(r.rev);
    }
    bool operator>(const revision& r) const{
        FILE_LOG(logDEBUG4) << "revision " << rev << " < " << r.rev << " == " << (abs(rev) < abs(r.rev));
        return abs(rev) > abs(r.rev);
    }
    bool operator<=(const revision& r) const{
        FILE_LOG(logDEBUG4) << "revision " << rev << " <= " << r.rev << " == " << (abs(rev) <= abs(r.rev));
        return abs(rev) <= abs(r.rev);
    }
    bool operator>=(const revision& r) const{
        return abs(rev) >= abs(r.rev);
    }
    bool operator==(const revision& r) const{
        FILE_LOG(logDEBUG4) << "revision " << rev << " == " << r.rev << " == " << (abs(rev) == abs(r.rev));
        return abs(rev) == abs(r.rev);
    }
    bool is_older(const revision& r)const{
        return *this < r;
    }
    revision& operator++(){
        FILE_LOG(logDEBUG2) << "increment revision";
        assert(rev >=0);
        ++rev;
        FILE_LOG(logDEBUG2) << "incremented";
        return *this;
    }
    revision& operator--(){
        FILE_LOG(logDEBUG2) << "decrement revision";
        assert(rev >0);
        --rev;
        FILE_LOG(logDEBUG2) << "decremented";
        return *this;
    }
    revision(int r) : rev(r){ }
//    operator int()
//    {
//         return rev;
//    }
    int get_rev() const {
        return rev;
    }

};

revision make_deleted(const revision& r){
    revision rev = r;
    rev.rev = -rev.rev;
    return rev;
}
bool is_deleted(const revision& rev){
    return rev.get_rev()<0;
}
std::ostream& operator<<(std::ostream& os, const revision& obj) {
    return os << obj.get_rev() << " ";
}



#endif // UTILS_H
