#include "simple_test.h"

using namespace std;
using namespace boost;
using testing::Types;

typedef boost::adjacency_list<vecS, vecS, undirectedS> SimpleGraphVec;

REGISTER_TYPED_TEST_CASE_P(GraphTest,integrity,simple);

typedef Types<SimpleGraphVec> GraphImplementations;
INSTANTIATE_TYPED_TEST_CASE_P(TestWithNoTypes, GraphTest, GraphImplementations);


