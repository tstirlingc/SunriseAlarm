#include "gtest/gtest.h"

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  int returnVal = RUN_ALL_TESTS();
  return returnVal;
}


