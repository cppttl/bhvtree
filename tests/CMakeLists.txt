cmake_minimum_required(VERSION 3.10)

set(TARGET_NAME ${CMAKE_PROJECT_NAME}-tests)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_PROJECT_NAME})

add_test(
    NAME    bhvtree::test
    COMMAND ${TARGET_NAME}
)
