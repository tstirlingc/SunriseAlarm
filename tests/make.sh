#!/bin/sh 

INCLUDE_DIRS="-I /Users/toddcoffey/Projects/arduino-mock/include/ -I /Users/toddcoffey/Projects/arduino-mock/lib/gtest/gtest/src/gtest/include/ -I /Users/toddcoffey/Projects/arduino-mock/lib/gmock/gmock/src/gmock/include -I /Users/toddcoffey/Documents/Arduino/libraries/RTClib -I /Applications/Arduino-1.6.12.app/Contents/Java/hardware/arduino/avr/cores/arduino"

#INCLUDE_DIRS="-I /Users/toddcoffey/Projects/arduino-mock/lib/gtest/gtest/src/gtest/include/" 

#LIB_DIRS="-L /Users/toddcoffey/Projects/arduino-mock/lib/gmock/gmock/src/gmock-build/ -L /Users/toddcoffey/Projects/arduino-mock/lib/gtest/gtest/src/gtest-build -L /Users/toddcoffey/Projects/arduino-mock/dist/lib -L /Users/toddcoffey/Projects/arduino-mock/lib/gmock/gmock/src/gmock-build/gtest/"
#LIBS="-l gmock -l arduino_mock -l gtest"
#LIBS="-l gtest_main -l gmock_main -l arduino_mock"
#/Users/toddcoffey/Projects/arduino-mock/build/lib/gmock/gmock/src/gmock-build/gtest/libgtest.a
BARE_LIBS="/Users/toddcoffey/Projects/arduino-mock/build/lib/gtest/gtest/src/gtest-build/libgtest.a /Users/toddcoffey/Projects/arduino-mock/build/lib/gmock/gmock/src/gmock-build/libgmock.a /Users/toddcoffey/Projects/arduino-mock/build/dist/lib/libarduino_mock.a"

#BARE_LIBS="/Users/toddcoffey/Projects/arduino-mock/build/lib/gtest/gtest/src/gtest-build/libgtest.a" 

c++ $INCLUDE_DIRS -c UnitTestClockTime.cpp
c++ $INCLUDE_DIRS -c UnitTestLightColor.cpp
c++ $INCLUDE_DIRS -c UnitTestMillisDelta.cpp
c++ $INCLUDE_DIRS main.cpp -o sunrise_tests UnitTestClockTime.o UnitTestLightColor.o UnitTestMillisDelta.o $BARE_LIBS

