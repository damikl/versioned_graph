#include "versioned_graph.h"

int main(){
    using namespace boost;
    using namespace std;
    FILE* log_fd = fopen( "/dev/null", "w" );
    Output2FILE::Stream() = log_fd;
    typedef versioned_graph<adjacency_list<boost::vecS, boost::vecS, boost::directedS,int,string>> simple_graph;
    simple_graph sg;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(1,sg);
    vertex_descriptor v2 = add_vertex(2,sg);
    vertex_descriptor v3 = add_vertex(3,sg);
    vertex_descriptor v4 = add_vertex(6,sg);
    add_edge(v1,v2,std::string("9"),sg);
    add_edge(v1,v3,"8",sg);
    add_edge(v2,v4,"7",sg);
    add_edge(v1,v4,"11",sg);
    add_edge(v2,v3,"12",sg);
    sg[v4] = 4;
    assert(4==num_vertices(sg));
    assert(5==num_edges(sg));
    cout << "start commit" << endl;
    commit(sg);
    cout << "commit end" << endl;
    sg[v4] = 5;
    remove_edge(v1,v4,sg);
    assert(4==num_edges(sg));
    assert(!edge(v1,v4,sg).second);
    commit(sg);
    sg[v4] = 6;
    undo_commit(sg);
    cout << "made undo" << endl;
    assert(edge(v1,v4,sg).second);
    cout << "edge recreated" << endl;
    assert(5==num_edges(sg));
    cout << "count match" << endl;
    assert(4==sg[v4]);
    cout << "attribute match" << endl;
    vertex_descriptor v5 = add_vertex(5,sg);
    add_edge(v5,v4,"13",sg);
    add_edge(v3,v5,"14",sg);
    assert(5==num_vertices(sg));
    assert(7==num_edges(sg));
    assert(5==sg[v5]);
    revert_changes(sg);
    assert(4==num_vertices(sg));
    assert(5==num_edges(sg));

    typedef versioned_graph<adjacency_list<boost::hash_setS, boost::hash_setS, boost::directedS,int,string>> hash_graph;
    hash_graph g;
    //typedef typename boost::graph_traits<hash_graph>::vertex_descriptor vertex_descr;
}
