cmake_minimum_required (VERSION 2.6)
project (intiTest)
# The version number.
set (intiTest_VERSION_MAJOR 0)
set (intiTest_VERSION_MINOR 1)

# GTest
ADD_SUBDIRECTORY(gtest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Unit Tests
add_executable( intiTest intiTest.cpp  )
# Link test executable against gtest & gtest_main
target_link_libraries(intiTest gtest gtest_main)

enable_testing()
add_test( intiTest intiTest  )
