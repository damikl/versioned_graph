/***
 * author: Damian Lipka
 *
 * */

#include "versioned_graph.h"
using namespace boost;
using namespace std;

/***
 * Przykłady zastosowania versioned_graph na potrzeby rozdziału 3.3.1
 * */

struct Details {
    int duration;
    int max_waiting_time;
    int start_time;
    Details(int dn, int max) : duration(dn),max_waiting_time(max),start_time(-1){}
    Details() : duration(0),max_waiting_time(0),start_time(-1){}

    bool operator==(const Details& o)const {
        return this->duration==o.duration && this->max_waiting_time==o.max_waiting_time;
    }
    bool operator!=(const Details& o)const {
        return !(*this==o);
    }
    void take(int start){
        start_time = start;
    }
    void clear(){
        start_time = -1;
    }
    bool is_reserved() const{
        return start_time >= 0;
    }
    bool is_in_conflict(int start, int end) const {
        bool result = is_reserved() && (max(start_time,start) < min(get_end_time(),end));
        if(result){
            cout << "Wystepuje konflikt: (" << start_time << " " << get_end_time()
             << ")  (" << start  << " " << end  << ") " << endl;
        }
        return result;
    }
    int get_end_time() const {
        return is_reserved() ? start_time + duration : 0;
    }
    string to_str(){
        return string("(").append(to_string(duration).append(" ").append(to_string(max_waiting_time).append(")")));
    }
};


typedef versioned_graph<adjacency_list<boost::listS, boost::listS, boost::undirectedS, Details>> simple_graph;
typedef typename boost::graph_traits<simple_graph>::vertex_descriptor vertex_descriptor;
vertex_descriptor a,b,c,d,f,g,h,i,j;

bool check_slot(const vertex_descriptor& v,int start, const simple_graph& sg){
    Details d = sg[v];
    int end = start + d.duration;
    cout << "Test: " << d.to_str() << " start w " << start << " minucie, koniec w " << end <<   endl;
    if(end > d.max_waiting_time){
        cout << "Zadanie " << d.to_str() << " zakonczy sie po: " << end << " minutach, czyli zbyt pozno" <<   endl;
        return false;
    }
    auto iter_p = adjacent_vertices(v,sg);
    for(auto v_it = iter_p.first; v_it!=iter_p.second; ++v_it){
        Details adj = sg[*v_it];
        if(adj.is_in_conflict(start,end)){
            cout << "Zadanie " << d.to_str() << " jest w konflikcie z zadaniem: " << adj.to_str() <<  endl;
            return false;
        }
    }
    return true;
}

bool contains(vector<vertex_descriptor>& v,vertex_descriptor& x){
    return std::find(v.begin(), v.end(), x) != v.end();
}

bool validate_state1(const simple_graph& sg){
	if(!edge(a,c,sg).second) return false;
    if(!edge(a,b,sg).second) return false;
    if(num_edges(sg)!=2) return false;
    if(num_vertices(sg)!=3) return false;
    if(sg[a].duration!=15) return false;
	if(sg[b].duration!=10) return false;
	if(sg[c].duration!=5) return false;
	return true;
}

bool validate_state2(const simple_graph& sg){
	if(!edge(d,f,sg).second) return false;
    if(!edge(d,g,sg).second) return false;
    if(num_edges(sg)!=5) return false;
    if(num_vertices(sg)!=6) return false;
    if(sg[d].duration!=20) return false;
	if(sg[f].duration!=10) return false;
	if(sg[g].duration!=5) return false;
	return true;
}

bool validate_state3(const simple_graph& sg){
	if(!edge(c,h,sg).second) return false;
    if(!edge(f,i,sg).second) return false;
    if(num_edges(sg)!=10) return false;
    if(num_vertices(sg)!=9) return false;
    if(sg[h].duration!=10) return false;
	if(sg[j].duration!=10) return false;
	if(sg[i].duration!=15) return false;
	return true;
}

void assign_task(simple_graph& sg, vector<string>& vec, vertex_descriptor& v, const string& label){
	int start = vec.size()*5;
	assert(check_slot(v,start,sg));
    sg[v].take(start);
	for(int i=0; i < (sg[v].duration/5.0f); ++i) { 
		vec.push_back(label);
	}
}
bool workplan_generator_demo(simple_graph& sg){
	vector<string> first, second;
	if(validate_state1(sg)) {
        assign_task(sg,first,b,string("B"));
        assign_task(sg,second,c,string("C"));
        assign_task(sg,first,a,string("A"));
	} else if(validate_state2(sg)){
		assign_task(sg,first,a,string("A"));
		assign_task(sg,second,f,string("F"));
		assign_task(sg,second,g,string("G"));
		assign_task(sg,first,d,string("D"));
		assign_task(sg,second,c,string("C"));
		assign_task(sg,second,b,string("B"));
	} else {
        cout << "Nie mozna znalezc poprawnego podzialu pracy" << endl;
		return false;
	}
    cout << "Pierwszy: ";
	for(auto& i : first){
        std::cout << i << ' ';
	}
	cout << endl << "Drugi:    ";
	for(auto& i : second){
        std::cout << i << ' ';
	}
	cout << endl;
    auto iter_p = vertices(sg);
    for(auto it =iter_p.first; it != iter_p.second; ++it){
        sg[*it].clear();
    }
	return true;
}

bool valid_workplan_exists(simple_graph& sg){
    // Pelny algorytm jest poza zakresem tej pracy
    return workplan_generator_demo(sg);
}

void create_state1(simple_graph& sg){
    a = add_vertex(Details(15,30),sg);
    b = add_vertex(Details(10,30),sg);
    c = add_vertex(Details(5,30),sg);
    add_edge(a,c,sg);
    add_edge(a,b,sg);
}

void create_state2(simple_graph& sg){
    d = add_vertex(Details(20,40),sg);
    f = add_vertex(Details(10,25),sg);
    g = add_vertex(Details(5,20),sg);
    add_edge(a,d,sg);
    add_edge(d,g,sg);
    add_edge(d,f,sg);
}

void create_state3(simple_graph& sg){
    h = add_vertex(Details(10,25),sg);
    j = add_vertex(Details(10,25),sg);
    i = add_vertex(Details(15,30),sg);
    add_edge(c,h,sg); add_edge(d,h,sg);
    add_edge(h,j,sg); add_edge(f,j,sg);
    add_edge(f,i,sg); add_edge(g,i,sg);
    clear_vertex(b,sg);
    remove_edge(b,sg);
}

int main(){

    simple_graph sg;

    create_state1(sg);
    bool result = valid_workplan_exists(sg);
    cout << "Wynik pierwszego etapu: " << (result ? "Podział udany" : "Podział nieudany" ) << endl;
    commit(sg);		// stan 1
    revert_changes(sg); // brak zmian

    create_state2(sg);
    result = valid_workplan_exists(sg);
    cout << "Wynik etapu drugiego: " << (result ? "Podział udany" : "Podział nieudany" ) << endl;
    // revert_changes(sg); verify_state1(sg);
    commit(sg); // stan 2

    create_state3(sg);
    result = valid_workplan_exists(sg);
    cout << "Wynik etapu trzeciego: " << (result ? "Podział udany" : "Podział nieudany" ) << endl;
    revert_changes(sg);
    assert(validate_state2(sg));
    cout << "Wycofano do stanu drugiego" << endl;



}

