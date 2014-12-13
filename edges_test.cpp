#include "edges_test.h"

using namespace std;
using namespace boost;
using testing::Types;

typedef boost::adjacency_list<vecS, vecS, undirectedS, extra_info,extra_info> ExtendedGraphVec;
typedef boost::adjacency_list<listS, listS, undirectedS, extra_info,extra_info> ExtendedGraphList;

typedef Types<ExtendedGraphVec> GraphImplementations;
INSTANTIATE_TYPED_TEST_CASE_P(TestWithAllTypes,EdgesGraphTest,GraphImplementations);
