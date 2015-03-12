#ifndef SIMPLE_TEST_H
#define SIMPLE_TEST_H
#include "tests.h"

TYPED_TEST_CASE_P(GraphTest);

TYPED_TEST_P(GraphTest, integrity) {
    FILELog::ReportingLevel() = logDEBUG4;
    ASSERT_TRUE(this->check());
}

TYPED_TEST_P(GraphTest, simple) {
    FILELog::ReportingLevel() = logDEBUG4;
    FILE_LOG(logINFO) << "Simple test " << std::endl;
    ASSERT_NO_FATAL_FAILURE(this->test());
    ASSERT_TRUE(this->check());
}

#endif // SIMPLE_TEST_H
