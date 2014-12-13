#ifndef VERTICES_TEST_H
#define VERTICES_TEST_H
#include "tests.h"

template <class T>
class VertexGraphTest : public GraphTest<T> {
};

TYPED_TEST_CASE_P(VertexGraphTest);

TYPED_TEST_P(VertexGraphTest, simple) {
    FILELog::ReportingLevel() = logDEBUG2;
    this->test();
    ASSERT_TRUE(this->check());
}

TYPED_TEST_P(VertexGraphTest, simpleWithRemove) {
    FILELog::ReportingLevel() = logDEBUG2;
    this->test();
    this->test_removal();
    ASSERT_TRUE(this->check());
}

TYPED_TEST_P(VertexGraphTest, attributeModification) {
    FILELog::ReportingLevel() = logDEBUG2;
    this->test();
    this->getGraph()[this->getVertex(5)].simple_name = "other";
    this->getGraph()[this->getVertex(3)].simple_name = "just something";
    this->commit();
    ASSERT_TRUE(this->check());
}

REGISTER_TYPED_TEST_CASE_P(VertexGraphTest,simple,simpleWithRemove,attributeModification);


#endif // VERTICES_TEST_H
