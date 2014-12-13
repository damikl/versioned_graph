#ifndef SIMPLE_TEST_H
#define SIMPLE_TEST_H
#include "tests.h"

TYPED_TEST_CASE_P(GraphTest);

TYPED_TEST_P(GraphTest, simple) {
    this->test();
    ASSERT_TRUE(this->check());
}

TYPED_TEST_P(GraphTest, simpleWithRemove) {
    this->test();
    this->test_removal();
    ASSERT_TRUE(this->check());
}

REGISTER_TYPED_TEST_CASE_P(GraphTest,simple,simpleWithRemove);

#endif // SIMPLE_TEST_H
