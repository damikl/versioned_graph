#include "simple_test.h"

using namespace std;
using namespace boost;
using testing::Types;

TYPED_TEST_P(GraphTest, simpleWithRemove) {
    FILELog::ReportingLevel() = logDEBUG4;
    FILE_LOG(logINFO) << "simpleWithRemove test " << std::endl;
    ASSERT_NO_FATAL_FAILURE(this->test());
    FILE_LOG(logINFO) << "test removal" << std::endl;
    ASSERT_NO_FATAL_FAILURE(this->test_removal());
    FILE_LOG(logINFO) << "check after removal" << std::endl;
    ASSERT_TRUE(this->check());
}

REGISTER_TYPED_TEST_CASE_P(GraphTest,integrity,simple,simpleWithRemove);
typedef boost::adjacency_list<listS, listS, undirectedS> SimpleGraphList;

typedef Types<SimpleGraphList> GraphImplementations;
INSTANTIATE_TYPED_TEST_CASE_P(TestWithNoAttributesTypes, GraphTest, GraphImplementations);
