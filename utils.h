#ifndef UTILS_H
#define UTILS_H

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
}

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

#endif // UTILS_H
