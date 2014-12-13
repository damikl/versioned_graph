#ifndef EDGES_TEST_H
#define EDGES_TEST_H
#include "tests.h"

template <class T>
class EdgesGraphTest : public GraphTest<T> {
};

TYPED_TEST_CASE_P(EdgesGraphTest);


TYPED_TEST_P(EdgesGraphTest, simple) {
    FILELog::ReportingLevel() = logDEBUG2;
    this->test();
    ASSERT_TRUE(this->check());
}

TYPED_TEST_P(EdgesGraphTest, withRemoval) {
    FILELog::ReportingLevel() = logDEBUG2;
    this->test();
    this->test_removal();
    this->commit();
    ASSERT_TRUE(this->check());
}

TYPED_TEST_P(EdgesGraphTest, attributeModification) {
    FILELog::ReportingLevel() = logDEBUG2;
    this->test();
    ASSERT_TRUE(this->check());
    this->getGraph()[this->getVertex(5)].simple_name = "other";
    this->getGraph()[this->getVertex(3)].simple_name = "just something";
    this->getGraph()[this->getEdge(0,3)].simple_name = "path weight";
    this->commit();
    ASSERT_TRUE(this->check());
}

REGISTER_TYPED_TEST_CASE_P(EdgesGraphTest,simple,withRemoval,attributeModification);

#endif // EDGES_TEST_H
