#include "vertices_test.h"

using namespace std;
using namespace boost;

TYPED_TEST_P(VertexGraphTest, verticesWithRemove) {
    FILELog::ReportingLevel() = logDEBUG2;
    FILE_LOG(logINFO) << "verticesWithRemove test " << std::endl;
    ASSERT_NO_FATAL_FAILURE(this->test());
    ASSERT_TRUE(this->same_as_head());
    FILE_LOG(logINFO) << "test removal" << std::endl;
    ASSERT_NO_FATAL_FAILURE(this->test_removal());
    ASSERT_TRUE(this->check());
}

REGISTER_TYPED_TEST_CASE_P(VertexGraphTest,simple,verticesWithRemove,attributeModification);

typedef boost::adjacency_list<listS, listS, undirectedS, extra_info> UndirectedGraphList;

INSTANTIATE_TYPED_TEST_CASE_P(TestWithVertexTypesList, VertexGraphTest, UndirectedGraphList);
