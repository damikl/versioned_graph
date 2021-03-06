/***
 * author: Damian Lipka
 *
 * */

#include "versioned_graph.h"
#include <stdlib.h>
#include <time.h>

/***
 * Przykłady zastosowania versioned_graph na potrzeby rozdziału 3.3.2
 * */

using namespace boost;
using namespace std;
typedef versioned_graph<adjacency_list<boost::listS, boost::listS, boost::directedS, int, int, string>> simple_graph;
typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;

vertex_descriptor a,b,c,d,e,f;

void create_state1(simple_graph& sg){
    a = add_vertex(1,sg);
    b = add_vertex(2,sg);
    c = add_vertex(3,sg);
    add_edge(a,b,7,sg);
    add_edge(a,c,6,sg);
    sg[graph_bundle] = "10%";
}
void verify_state1(const simple_graph& sg){
    assert(edge(a,b,sg).second);
    assert(edge(a,c,sg).second);
    assert(num_edges(sg)==2);
    assert(num_vertices(sg)==3);
    assert(sg[edge(a,b,sg).first]==7);
    assert(sg[edge(a,c,sg).first]==6);
    assert(sg[graph_bundle]=="10%");
    cout << "Graf jest w stanie 1" << endl;
}

void create_state2(simple_graph& sg){
    remove_edge(edge(a,b,sg).first,sg);
    remove_vertex(b,sg);
    d = add_vertex(4,sg);
    e = add_vertex(5,sg);
    add_edge(d,e,2,sg);
    add_edge(e,d,3,sg);
    add_edge(c,c,7,sg);
    sg[graph_bundle] = "30%";
}

void verify_state2(const simple_graph& sg){
    assert(edge(d,e,sg).second);
    assert(edge(e,d,sg).second);
    assert(!edge(a,b,sg).second);
    assert(num_edges(sg)==4);
    assert(num_vertices(sg)==4);
    assert(sg[edge(c,c,sg).first]==7);
    assert(sg[graph_bundle]=="30%");
    cout << "Graf jest w stanie 2" << endl;
}

void create_state3(simple_graph& sg){
    clear_vertex(e,sg);
    remove_vertex(e,sg);
    f = add_vertex(6,sg);
    sg[edge(c,c,sg).first] = 9;
    add_edge(d,f,8,sg);
    sg[a] = 4; sg[c] = 5;
    sg[graph_bundle] = "60%";
}

void verify_state3(const simple_graph& sg){
    assert(num_edges(sg)==3);
    assert(num_vertices(sg)==4);
    assert(sg[edge(c,c,sg).first]==9);
    assert(sg[graph_bundle]=="60%");
    cout << "Graf jest w stanie 3" << endl;
}

void create_state4(simple_graph& sg){
    sg[edge(a,c,sg).first] = 1;
    add_edge(c,f,3,sg);
    sg[d] = 7;
}
void verify_state4(const simple_graph& sg){
    assert(edge(c,f,sg).second);
    assert(num_edges(sg)==4);
    assert(num_vertices(sg)==4);
    assert(sg[edge(a,c,sg).first]==1);
    assert(sg[edge(c,f,sg).first]==3);
    assert(sg[d]==7);
    cout << "Graf jest w stanie 4" << endl;
}

int main(){

    simple_graph sg;

    create_state1(sg);
    verify_state1(sg);
    commit(sg);		// stan 1
    revert_changes(sg); // brak zmian
    verify_state1(sg);


    create_state2(sg);
    verify_state2(sg);
    // revert_changes(sg); verify_state1(sg);
    commit(sg); // stan 2
    revert_changes(sg); // brak zmian
    verify_state2(sg);


    create_state3(sg);
    // revert_changes(sg); verify_state2(sg);
    // undo_commit(sg); verify_state1(sg);
    commit(sg); // stan 3
    verify_state3(sg);
    // undo_commit(sg);  verify_state2(sg);

    create_state4(sg);
    verify_state4(sg);

    srand ( time(NULL) );
    if(rand() % 2 == 1){
        cout << "Wybrano revert do stanu 3" << endl;
        revert_changes(sg); // stan 3
        verify_state3(sg);
        revert_changes(sg); // dalej stan 3
        verify_state3(sg);
    }
    undo_commit(sg);
    verify_state2(sg);
    undo_commit(sg);
    verify_state1(sg);



}

