#ifndef UTILS_H
#define UTILS_H
#include "log.h"

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
}

std::ostream& operator<<(std::ostream& os, const internal_vertex& obj) {
    return os << obj.identifier << " ";
}

struct revision{
    int rev;
    bool operator<(const revision& r) const{
        return abs(rev) < abs(r.rev);
    }
    bool operator==(const revision& r) const{
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
    revision(int r) : rev(r){ }
    operator int()
    {
         return rev;
    }
    int get_rev() const {
        return rev;
    }

};

std::ostream& operator<<(std::ostream& os, const revision& obj) {
    return os << obj.get_rev() << " ";
}



#endif // UTILS_H
