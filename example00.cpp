/***
 * author: Damian Lipka
 *
 * */

#include <boost/graph/adjacency_list.hpp>
#include <string>
using namespace boost;
using namespace std;

/***
 * Przykłady użycia adjacency_list oraz atrybutów na potrzeby rozdziału 2.4
 * */

void simple_vecs(){
    typedef adjacency_list<boost::vecS, boost::vecS, boost::directedS> simple_graph;
    simple_graph sg;
    add_edge(1,3,sg);
    add_edge(2,4,sg);
    add_edge(1,4,sg);
    add_edge(2,3,sg);
}

void simple_lists(){
    typedef adjacency_list<boost::listS, boost::listS, boost::directedS> simple_graph;
    simple_graph sg;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(sg);
    vertex_descriptor v2 = add_vertex(sg);
    vertex_descriptor v3 = add_vertex(sg);
    vertex_descriptor v4 = add_vertex(sg);
    typedef typename boost::graph_traits<simple_graph>::edge_descriptor edge_descriptor;
    edge_descriptor e;
    bool result;
    add_edge(v1,v3,sg);
    add_edge(v2,v4,sg);
    add_edge(v1,v4,sg);
    boost::tie(e,result) = add_edge(v2,v3,sg); // result == true
    assert(result);
    boost::tie(e,result) = add_edge(v2,v3,sg); // result == true
    assert(result);
}

void attributes(){
    typedef adjacency_list<boost::listS, boost::listS, boost::directedS, int, string, int> simple_graph;
    simple_graph sg;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(1,sg); // utworzenie wierzchołka
    vertex_descriptor v2 = add_vertex(2,sg);	// określonym atrybutem
    vertex_descriptor v3 = add_vertex(3,sg);
    vertex_descriptor v4 = add_vertex(6,sg);
    add_edge(v1,v2,"A1",sg);	// utworzenie krawędzi z określonym atrybutem
    add_edge(v1,v3,"A2",sg);
    add_edge(v2,v4,"A3",sg);
    add_edge(v1,v4,"A4",sg);
    add_edge(v2,v3,"A5",sg);
    sg[v4] = 4;	// przypisanie atrybutu wierzchołka
    typedef typename boost::graph_traits<simple_graph>::edge_descriptor edge_descriptor;

    edge_descriptor e = edge(v2,v4,sg).first;
    sg[e] = "A6";	// przypisanie atrybutu krawędzi
    sg[graph_bundle] = 30;	// przypisanie atrybutu grafu
    add_edge(v2,v4,"A6",sg);
    typedef typename boost::graph_traits<simple_graph>::out_edge_iterator out_edge_iterator;
    pair<out_edge_iterator, out_edge_iterator> ei = out_edges(v2,sg);
    for(out_edge_iterator edge_iter = ei.first;edge_iter != ei.second;++edge_iter){
        if(target(*edge_iter,sg)==v4){
            sg[*edge_iter] = "A7";
        } else {
            sg[*edge_iter].append("B");
        }
    }
}

struct Highway
{
  string name;
  double miles;
  int speed_limit;
  int lanes;
  Highway(): name(""),miles(0.0),speed_limit(120),lanes(2){}
  Highway(const string& name,int miles ): name(name),miles(miles),speed_limit(120),lanes(2){}
};

void complex_attributes(){
    typedef adjacency_list<boost::listS, boost::listS, boost::directedS, int, Highway> simple_graph;
    simple_graph sg;
    typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
    vertex_descriptor v1 = add_vertex(1,sg); // utworzenie wierzchołka
    vertex_descriptor v2 = add_vertex(2,sg);	// określonym atrybutem
    vertex_descriptor v3 = add_vertex(3,sg);
    vertex_descriptor v4 = add_vertex(6,sg);
    add_edge(v1,v2,sg);	// utworzenie krawędzi z domyslnym atrybutem
    add_edge(v1,v3,Highway(string("A4"),70),sg); // utworzenie krawędzi z okreslonym atrybutem
    add_edge(v2,v4,sg);
    add_edge(v1,v4,sg);
    add_edge(v2,v3,sg);
    sg[v4] = 4;	// przypisanie atrybutu wierzchołka
    typedef typename boost::graph_traits<simple_graph>::edge_descriptor edge_descriptor;

    edge_descriptor e = edge(v1,v3,sg).first;
    sg[e].name = "A1";
    typedef typename boost::graph_traits<simple_graph>::out_edge_iterator out_edge_iterator;
    pair<out_edge_iterator, out_edge_iterator> ei = out_edges(v2,sg);
    for(out_edge_iterator edge_iter = ei.first;edge_iter != ei.second;++edge_iter){
        if(target(*edge_iter,sg)==v4){
            sg[*edge_iter].lanes = 3;
        } else {
            sg[*edge_iter].speed_limit += 20;
        }
    }

}

int main(){
    simple_vecs();
    simple_lists();
    attributes();
    complex_attributes();
}


