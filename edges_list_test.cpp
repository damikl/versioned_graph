#include "edges_test.h"

using namespace std;
using namespace boost;
using testing::Types;

typedef boost::adjacency_list<vecS, vecS, undirectedS, extra_info,extra_info> ExtendedGraphVec;
typedef boost::adjacency_list<listS, listS, undirectedS, extra_info,extra_info> ExtendedGraphList;

TYPED_TEST_P(EdgesGraphTest, withRemoval) {
    FILELog::ReportingLevel() = logDEBUG2;
    ASSERT_NO_FATAL_FAILURE(this->test());
    ASSERT_NO_FATAL_FAILURE(this->test_removal());
    this->commit();
    ASSERT_TRUE(this->check());
}

REGISTER_TYPED_TEST_CASE_P(EdgesGraphTest,simple,withRemoval,attributeModification);

typedef Types<ExtendedGraphList> GraphImplementations;
INSTANTIATE_TYPED_TEST_CASE_P(TestWithAllTypesList,EdgesGraphTest,GraphImplementations);
