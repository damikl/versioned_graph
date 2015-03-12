#include "vertices_test.h"

using namespace std;
using namespace boost;
using testing::Types;

REGISTER_TYPED_TEST_CASE_P(VertexGraphTest,simple,attributeModification);

typedef boost::adjacency_list<vecS, vecS, undirectedS, extra_info> UndirectedGraphVec;

typedef Types<UndirectedGraphVec> GraphImplementations;
INSTANTIATE_TYPED_TEST_CASE_P(TestWithVertexTypes, VertexGraphTest, GraphImplementations);
