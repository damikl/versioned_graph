#include "vertices_test.h"

using namespace std;
using namespace boost;

typedef boost::adjacency_list<listS, listS, undirectedS, extra_info> UndirectedGraphList;

INSTANTIATE_TYPED_TEST_CASE_P(TestWithVertexTypesList, VertexGraphTest, UndirectedGraphList);
