#include "versioned_graph.h"

int main(){
    using namespace boost;
    using namespace std;
    typedef versioned_graph<adjacency_list<boost::vecS, boost::listS, boost::directedS,int,string>> simple_graph;
    simple_graph sg;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor A = add_vertex(1,sg);
    vertex_descriptor B = add_vertex(2,sg);
    vertex_descriptor C = add_vertex(3,sg);
    vertex_descriptor D = add_vertex(6,sg);
    add_edge(A,B,std::string("9"),sg);
    add_edge(A,C,"8",sg);
    add_edge(B,D,"7",sg);
    add_edge(A,D,"11",sg);
    add_edge(B,C,"12",sg);
    sg[D] = 4;
    assert(4==num_vertices(sg));
    assert(5==num_edges(sg));
    cout << "start commit" << endl;
    commit(sg);
    cout << "commit end" << endl;
    sg[D] = 5;
    remove_edge(A,D,sg);
    assert(4==num_edges(sg));
    assert(!edge(A,D,sg).second);
    commit(sg);
    sg[D] = 6;
    undo_commit(sg);
    cout << "made undo" << endl;
    assert(edge(A,D,sg).second);
    cout << "edge recreated" << endl;
    assert(5==num_edges(sg));
    cout << "count match" << endl;
    assert(4==sg[D]);
    cout << "attribute match" << endl;
    vertex_descriptor E = add_vertex(5,sg);
    add_edge(E,D,"13",sg);
    add_edge(C,E,"14",sg);
    assert(5==num_vertices(sg));
    assert(7==num_edges(sg));
    assert(5==sg[E]);
    revert_changes(sg);
    assert(4==num_vertices(sg));
    assert(5==num_edges(sg));

    typedef versioned_graph<adjacency_list<boost::hash_setS, boost::hash_setS, boost::directedS,int,string>> hash_graph;
    hash_graph g;
    //typedef typename boost::graph_traits<hash_graph>::vertex_descriptor vertex_descr;
}
