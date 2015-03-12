#ifndef EDGES_TEST_H
#define EDGES_TEST_H
#include "tests.h"

template <class Graph>
class EdgesGraphTest : public GraphTest<Graph> {
    typedef archive_handle<Graph> handle_type;
};

TYPED_TEST_CASE_P(EdgesGraphTest);


TYPED_TEST_P(EdgesGraphTest, simple) {
    FILELog::ReportingLevel() = logDEBUG2;
    ASSERT_NO_FATAL_FAILURE(this->test());
    ASSERT_TRUE(this->check());
}

TYPED_TEST_P(EdgesGraphTest, attributeModification) {
    FILELog::ReportingLevel() = logDEBUG3;
    ASSERT_NO_FATAL_FAILURE(this->test());
    ASSERT_TRUE(this->check());
    this->getGraph()[this->getVertex(5)].simple_name = "other";
    this->getGraph()[this->getVertex(3)].simple_name = "just something";
    this->getGraph()[this->getEdge(0,3)].simple_name = "path weight";
    FILE_LOG(logDEBUG1) << "attributes changed";
    ASSERT_TRUE(this->check());
    FILE_LOG(logDEBUG1) << "data consistent";
    ASSERT_NO_FATAL_FAILURE(this->commit());
    ASSERT_TRUE(this->check());
}

#endif // EDGES_TEST_H
