#include "vertices_test.h"

using namespace std;
using namespace boost;

TYPED_TEST_P(VertexGraphTest, simpleWithRemove) {
    FILELog::ReportingLevel() = logDEBUG2;
    ASSERT_NO_FATAL_FAILURE(this->test());
    ASSERT_NO_FATAL_FAILURE(this->test_removal());
    ASSERT_TRUE(this->check());
}

REGISTER_TYPED_TEST_CASE_P(VertexGraphTest,simple,simpleWithRemove,attributeModification);

typedef boost::adjacency_list<listS, listS, undirectedS, extra_info> UndirectedGraphList;

INSTANTIATE_TYPED_TEST_CASE_P(TestWithVertexTypesList, VertexGraphTest, UndirectedGraphList);
