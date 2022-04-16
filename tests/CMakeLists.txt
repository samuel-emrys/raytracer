# Obtain dependencies
find_package(
  GTest REQUIRED
  COMPONENTS gtest
             gmock
             gtest_main
             gmock_main)

set(EXTERNAL_DEV_LIBS
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main)

# Identify source files
file(
  GLOB_RECURSE
  TEST_SOURCES
  CONFIGURE_DEPENDS
  "*.cpp")

# Construct test executable
add_executable(${PROJECT_TEST} ${TEST_SOURCES})
target_link_libraries(
  ${PROJECT_TEST}
  PRIVATE project_options
          project_warnings
          ${EXTERNAL_DEV_LIBS}
          ${PROJECT_LIB})

add_test(
  NAME ${PROJECT_TEST}
  COMMAND ${PROJECT_TEST})

