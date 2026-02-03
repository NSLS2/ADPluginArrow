#ifndef TEST_NDFILEARROW_H
#define TEST_NDFILEARROW_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>

#include "NDFileArrow.h"

using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::StrictMock;


class TestNDFileArrow : public ::testing::Test {
   protected:
    void SetUp() override {
        // This function is called before each test is run
        // arrowPlugin = new NDFileArrow(getUniquePortName().c_str(), 10, 0, "NDArrayPort", 0, 5, 1024 * 1024, 0, 0, 1);
    }

    void TearDown() override {
        // This function is called after each test is run
        // delete this->arrowPlugin;
    }


    std::string getUniquePortName() {
        static int portCounter = 0;
        return "ARROW" + std::to_string(portCounter++);
    }


    // NDFileArrow* arrowPlugin;
};
#endif // TEST_NDFILEARROW_H