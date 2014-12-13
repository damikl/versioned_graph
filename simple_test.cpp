#include "simple_test.h"

using namespace std;
using namespace boost;
using testing::Types;

typedef boost::adjacency_list<vecS, vecS, undirectedS> SimpleGraphVec;
typedef boost::adjacency_list<listS, listS, undirectedS> SimpleGraphList;

typedef Types<SimpleGraphVec> GraphImplementations;
INSTANTIATE_TYPED_TEST_CASE_P(TestWithNoTypes, GraphTest, GraphImplementations);


