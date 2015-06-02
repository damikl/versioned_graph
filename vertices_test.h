#ifndef VERTICES_TEST_H
#define VERTICES_TEST_H
#include "tests.h"

template <class T>
class VertexGraphTest : public GraphTest<T> {
};

TYPED_TEST_CASE_P(VertexGraphTest);

TYPED_TEST_P(VertexGraphTest, simple) {
    FILELog::ReportingLevel() = logDEBUG4;
    ASSERT_NO_FATAL_FAILURE(this->test());
    ASSERT_TRUE(this->check());
}

TYPED_TEST_P(VertexGraphTest, attributeModification) {
    FILELog::ReportingLevel() = logDEBUG4;
    ASSERT_NO_FATAL_FAILURE(this->test());
    ASSERT_TRUE(this->same_as_head());
    this->getGraph()[this->getVertex(5)].simple_name = "other";
    this->getGraph()[this->getVertex(3)].simple_name = "just something";
    FILE_LOG(logDEBUG1) << "attributes changed";
    ASSERT_FALSE(this->same_as_head());
    ASSERT_TRUE(this->check());
    FILE_LOG(logDEBUG1) << "data consistent";
    ASSERT_NO_FATAL_FAILURE(this->commit());
    ASSERT_TRUE(this->check());
}

#endif // VERTICES_TEST_H
